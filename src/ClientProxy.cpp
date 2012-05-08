#include "ClientProxy.h"

#include "Node.h"

#include <reef/IActiveLink.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <map>

namespace reef {

/*
 Maps a all the instance Ids belonging to a host to their local Remote objects representations.
 */
class Host
{
public:
    inline void addInstance( ClientProxy* remoteObj, co::int32 instanceID )
    {
        _instances.insert( std::pair<co::int32, ClientProxy*>( instanceID, remoteObj ) );
    }
    
    inline ClientProxy* getInstance( co::int32 instanceID )
    {
        InstanceMap::iterator it = _instances.find( instanceID );
        return it != _instances.end() ? it->second : 0;
    }
    
    inline void removeInstance( co::int32 instanceID )
    {
        InstanceMap::iterator it = _instances.find( instanceID );
        _instances.erase( it );
    }
    
    inline bool hasInstance()
    {
        return _instances.size() > 0;
    }
private:
    typedef std::map<co::int32, ClientProxy*> InstanceMap;
    InstanceMap _instances;
};

// Maps every Connecter to a Host. Could be ip to Host but would be slower.
typedef std::map<IActiveLink*, Host*> Hosts;
static Hosts _hosts;
    
ClientProxy* ClientProxy::getOrCreateClientProxy( Node* node, co::IComponent* component,
                                            IActiveLink* link, co::int32 instanceID )
{
    Host* host = 0;
    ClientProxy* retValue = 0;
    Hosts::iterator it = _hosts.find( link );
    if( it != _hosts.end() )
    {
        host = it->second;
        retValue = host->getInstance( instanceID );
        
        if( retValue )
            return retValue;
    }
    else
    {
        host = new Host();
        _hosts.insert( std::pair<IActiveLink*, Host*>( link, host ) );
    }
    
    retValue = new ClientProxy( node, component, link, instanceID );
    host->addInstance( retValue, instanceID );
    
    return retValue;
}
    
ClientProxy::ClientProxy()
{
}
    
ClientProxy::ClientProxy( Node* node, co::IComponent* component, IActiveLink* link, 
                           co::int32 instanceID ) : _node( node ), _link( link ),_numFacets( 0 )
{
    _classPtr = *reinterpret_cast<void**>( this );
    setComponent( component );
    _instanceID = instanceID;
}
    
ClientProxy::~ClientProxy()
{   
    Hosts::iterator it = _hosts.find( _link.get() );
    assert( it != _hosts.end() );
    
    Host* host = it->second;
    host->removeInstance( _instanceID );
    
    if( !host->hasInstance() )
    {
        delete host;
        _hosts.erase( it );
    }
    
    for( int i = 0; i < _numFacets; i++ )
    {
        delete _facets[i];
    }
    delete [] _facets;
    
    _node->requestEndAccess( _link.get(), _instanceID, "TODO" );
}
    
void ClientProxy::setComponent( co::IComponent* component )
{    
	_component = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _component->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets = new co::IService*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
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
    
    return reef::ClientProxy_Base::getServiceAt( port );
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
    _marshaller.beginCallMarshalling( _instanceID, dynFacetId, field->getIndex(), true );
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _link->send( msg );
    
    awaitReplyUpdating( msg );
    
    _unmarshaller.unmarshalData( msg, field->getType(), _resultBuffer );

    return _resultBuffer;
}
    
void ClientProxy::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    _marshaller.beginCallMarshalling( _instanceID, dynFacetId, field->getIndex(), false );
    
    if( value.getKind() != co::TK_INTERFACE )
        _marshaller.marshalValueParam( value );
    else
        onInterfaceParam( value.get<co::IService*>() );

    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _link->send( msg );

}

const co::Any& ClientProxy::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, 
                                           co::Range<co::Any const> args )
{
    co::IType* returnType = method->getReturnType();
    if( returnType )
        _marshaller.beginCallMarshalling( _instanceID, dynFacetId, method->getIndex(), true );
    else
        _marshaller.beginCallMarshalling( _instanceID, dynFacetId, method->getIndex(), false );
    
    for( ; args; args.popFirst() )
    {
        const co::Any& arg = args.getFirst();
        
        if( arg.getKind() != co::TK_INTERFACE )
            _marshaller.marshalValueParam( arg );
        else
            onInterfaceParam( arg.get<co::IService*>() );
    }
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _link->send( msg );
    
	if( returnType )
    {
        awaitReplyUpdating( msg );
        _unmarshaller.unmarshalData( msg, method->getReturnType(), _resultBuffer );
    }

    return _resultBuffer;
}

co::int32 ClientProxy::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}
     
void ClientProxy::onInterfaceParam( co::IService* param )
{
    co::IObject* provider = param->getProvider();
    std::string providerType = provider->getComponent()->getFullName();
    co::int32 facetIdx = param->getFacet()->getIndex();
    co::int32 instanceID;
    
    if( isLocalObject( provider ) )
    {
        instanceID = _node->publishAnonymousInstance( provider );
        _marshaller.marshalRefParam( instanceID, facetIdx, Marshaller::RefOwner::LOCAL, &providerType, 
                             &_node->getPublicAddress() );
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerRO = static_cast<ClientProxy*>( provider );
        IInstanceInfo* info = static_cast<IInstanceInfo*>( providerRO );
        
        instanceID = info->getInstanceID();
        const std::string& ownerAddress = info->getOwnerAddress();
        
        if( ownerAddress == _link->getAddress() ) // Receiver
        {
            _marshaller.marshalRefParam( instanceID, facetIdx, Marshaller::RefOwner::RECEIVER );
        }
        else
        {
            _node->requestBeginAccess( ownerAddress, instanceID, "TODO" );
            _marshaller.marshalRefParam( instanceID, facetIdx, Marshaller::RefOwner::ANOTHER, &providerType, 
                                 &ownerAddress );
        }
    }
}

    // ------ reef.IInstanceInfo Methods ------ //
    
co::int32 ClientProxy::getInstanceID()
{
    return _instanceID;
}

const std::string& ClientProxy::getOwnerAddress()
{
    return _link->getAddress();
}
    
void ClientProxy::awaitReplyUpdating( std::string& msg )
{
    while( !_link->receiveReply( msg ) )
        _node->update();

}

CORAL_EXPORT_COMPONENT( ClientProxy, ClientProxy );
    
} // namespace reef
