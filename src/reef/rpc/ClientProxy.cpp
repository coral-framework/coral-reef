#include "ClientProxy.h"

#include "Node.h"

#include <reef/rpc/IActiveLink.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <map>

namespace reef {
namespace rpc {

    
ClientProxy::ClientProxy()
{
}
    
ClientProxy::ClientProxy( Requestor* requestor, co::IComponent* component, co::int32 instanceID ) : 
    _requestor( requestor ), _numFacets( 0 ), _instanceID( instanceID )
{
    s_classPtr = *reinterpret_cast<void**>( this );
    setComponent( component );
    _instanceID = instanceID;
}
    
ClientProxy::~ClientProxy()
{   
    for( int i = 0; i < _numFacets; i++ )
    {
        delete _facets[i];
    }
    delete [] _interfaces;
    delete [] _facets;
    
    _requestor->requestLeaseBreak( _instanceID );
}
    
void ClientProxy::setComponent( co::IComponent* component )
{    
	_component = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _component->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets = new co::IService*[numFacets];
    _interfaces = new co::IInterface*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
        _interfaces[i] = facets[i]->getType();
	}
}

co::IComponent* ClientProxy::getComponent()
{
    return _component;
}
    
co::IService* ClientProxy::getServiceAt( co::IPort* port )
{
    co::ICompositeType* owner = port->getOwner();
    if( owner == _component )
    {
        int portIndex = port->getIndex();
        assert( portIndex <= _numFacets );
        return _facets[portIndex];
    }
    
    return reef::rpc::ClientProxy_Base::getServiceAt( port );
}

void ClientProxy::setServiceAt( co::IPort* receptacle, co::IService* instance )
{
    // empty: setting remote services not supported yet
}

co::IPort* ClientProxy::dynamicGetFacet( co::int32 cookie )
{
    return _component->getFacets()[cookie];
}
        
const co::Any& ClientProxy::dynamicGetField( co::int32 dynFacetId, co::IField* field )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], field->getOwner() );
    
    _marshaller.beginCallMarshalling( _instanceId, dynFacetId, field->getIndex(), depth, true,
                                     _node->getPublicAddress() );
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _link->send( msg );
    
    awaitReplyUpdating( msg );
    
    demarshalReturn( msg, field->getType(), _resultBuffer );

    return _resultBuffer;
}
    
void ClientProxy::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], field->getOwner() );
    
    _marshaller.beginCallMarshalling( _instanceId, dynFacetId, field->getIndex(), depth, false ,
                                     _node->getPublicAddress() );

    
    if( value.getKind() != co::TK_INTERFACE )
        _marshaller.addValueParam( value );
    else
        onInterfaceParam( value.get<co::IService*>() );

    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _link->send( msg );

}

const co::Any& ClientProxy::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, 
                                           co::Range<co::Any const> args )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], method->getOwner() );
    
    co::IType* returnType = method->getReturnType();

    
    return _resultBuffer;
}

co::int32 ClientProxy::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}

void ClientProxy::demarshalReturn( const std::string& data, co::IType* returnedType, 
                                  co::Any& returned )
{
    if( returnedType->getKind() != co::TK_INTERFACE )
    {
        _demarshaller.demarshalValue( data, returnedType, returned );
        return;
    }
    
    co::int32 instanceId;
    co::int32 facetIdx;
    Demarshaller::RefOwner owner;
    std::string instanceType;
    std::string ownerAddress;
    
    _demarshaller.demarshalReference( data, instanceId, facetIdx, owner, instanceType, ownerAddress );
    co::IObject* instance;
    switch( owner )
    {
        case Demarshaller::RefOwner::RECEIVER:
            instance = _node->getInstance( instanceId );
            break;
        case Demarshaller::RefOwner::LOCAL:
        case Demarshaller::RefOwner::ANOTHER:
            instance = _node->getRemoteInstance( instanceType, instanceId, 
                                                ownerAddress );
            break;
    }
    _tempRef = instance; // TODO: remove
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    co::IService* service = instance->getServiceAt( port );
    returned.set<co::IService*>( service );
}

void ClientProxy::onInterfaceParam( co::IService* param )
{
    co::IObject* provider = param->getProvider();
    std::string providerType = provider->getComponent()->getFullName();
    co::int32 facetIdx = param->getFacet()->getIndex();
    co::int32 instanceId;
    
    if( isLocalObject( provider ) )
    {
        instanceId = _node->publishAnonymousInstance( provider, _address );
        _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::LOCAL, &providerType, 
                             &_node->getPublicAddress() );
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        instanceId = providerCP->getInstanceId();
        const std::string instanceOwnerAddress = providerCP->getOwnerAddress();
        
        if( instanceOwnerAddress == _address ) // Receiver
        {
            _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::RECEIVER );
        }
        else
        {
            _node->requestBeginAccess( instanceOwnerAddress, instanceId, _address );
            _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::ANOTHER, &providerType,
                                 &instanceOwnerAddress );
        }
    }
}

    // ------ reef.rpc.IInstanceInfo Methods ------ //
    
co::int32 ClientProxy::getInstanceId()
{
    return _instanceId;
}

const std::string& ClientProxy::getOwnerAddress()
{
    return _address;
}
    
void ClientProxy::awaitReplyUpdating( std::string& msg )
{
    while( !_link->receiveReply( msg ) )
        _node->update();

}

co::int32 ClientProxy::findDepth( co::IInterface* facet, co::ICompositeType* memberOwner )
{
    if( memberOwner == facet )
        return -1;

    co::Range<co::IInterface* const> superTypes( facet->getSuperTypes() );
    for( int i = 0; superTypes; superTypes.popFirst(), i++ )
    {
        if( superTypes.getFirst() == memberOwner )
            return i;
    }
    return -2;
}
    
CORAL_EXPORT_COMPONENT( ClientProxy, ClientProxy );
    
    
}
    
 
} // namespace reef
