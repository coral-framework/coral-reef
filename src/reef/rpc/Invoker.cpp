#include "Invoker.h"

#include "Node.h"
#include "ClientProxy.h"

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IMember.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <string>
#include <iostream>

namespace reef {
namespace rpc {


Invoker::Invoker( Node* node, co::IObject* object ) : _node( node )
{
    if( object )
    {
        _object = object;
        _component = _object->getComponent();
        co::Range<co::IPort* const> ports = _component->getFacets();
        co::int32 numPorts = static_cast<co::int32>( ports.getSize() );
        _openedServices.resize( numPorts );
        _openedInterfaces.resize( numPorts );
        _openedReflectors.resize( numPorts );
    }
}
    
Invoker::~Invoker()
{

}
    
void Invoker::invoke( Unmarshaller& unmarshaller, bool isSynch, std::string& returned )
{
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::Any parameter;
    co::int32 depth;
    std::string caller;
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, depth, caller );
    
    if( !_openedServices[facetIdx] ) // if already used before access directly
        onServiceFirstAccess( facetIdx );
    
    co::IService* facet = _openedServices[facetIdx];
    co::IInterface* memberOwner = _openedInterfaces[facetIdx];
    
    // if method is inherited
    if( depth != -1 )
        memberOwner = memberOwner->getSuperTypes()[depth];
        
    co::IReflector* reflector = memberOwner->getReflector();
    co::IMember* member = memberOwner->getMembers()[memberIdx];
    co::MemberKind kind = member->getKind();
    
    if( kind == co::MK_METHOD )
    {
        onMethod( unmarshaller, facet, co::cast<co::IMethod>( member ), reflector, parameter );
        if( !isSynch )
            return;
    }   
    else   
    {
        if( !isSynch )
        {
            onSetField( unmarshaller, facet, co::cast<co::IField>( member ), reflector );
            return;
        }
        else
        {
            onGetField( unmarshaller, facet, co::cast<co::IField>( member ), reflector, parameter );
        }
    }
   
    // Marshals the return from the call
    if( parameter.getKind() != co::TK_INTERFACE )
        _marshaller.marshalValueType( parameter, returned );
    else
        onInterfaceReturned( parameter.get<co::IService*>(), caller, returned );
}

void Invoker::onMethod( Unmarshaller& unmarshaller, co::IService* facet, co::IMethod* method, 
                           co::IReflector* refl, co::Any& returned )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    std::vector<co::Any> args;
    co::Range<co::IParameter* const> params = method->getParameters(); 
    
    size_t size = params.getSize();
    args.resize( size );
    for( int i = 0; i < size; i++ )
    {
        unmarshalParameter( unmarshaller, params[i]->getType(), args[i], tempReferences );
    }

    refl->invoke( facet, method, args, returned );
}
    
void Invoker::onGetField( Unmarshaller& unmarshaller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl, co::Any& returned )
{
    refl->getField( facet, field, returned );
}

void Invoker::onSetField(  Unmarshaller& unmarshaller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    co::Any value;
    unmarshalParameter( unmarshaller, field->getType(), value, tempReferences );
    
    refl->setField( facet, field, value );
}
    
void Invoker::unmarshalParameter( Unmarshaller& unmarshaller, co::IType* paramType, co::Any& param, 
                         co::RefVector<co::IObject>& tempRefs )
{
    if( paramType->getKind() != co::TK_INTERFACE )
    {
        unmarshaller.unmarshalValueParam( param, paramType );
        return;
    }
    
    co::int32 instanceId;
    co::int32 facetIdx;
    Unmarshaller::RefOwner owner;
    std::string instanceType;
    std::string ownerAddress;
    
    unmarshaller.unmarshalReferenceParam( instanceId, facetIdx, owner, instanceType, ownerAddress );
    co::IObject* instance;
    switch( owner )
    {
        case Unmarshaller::RECEIVER:
            instance = _node->getInstance( instanceId );
            break;
        case Unmarshaller::LOCAL:
        case Unmarshaller::ANOTHER:
            instance = _node->getRemoteInstance( instanceType, instanceId, 
                                                     ownerAddress );
            break;
    }
    tempRefs.push_back( instance ); // TODO: remove
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    co::IService* service = instance->getServiceAt( port );
    param.set<co::IService*>( service );
}

void Invoker::onInterfaceReturned( co::IService* returned, std::string& caller, 
                                      std::string& marshalledReturn )
{
    co::IObject* provider = returned->getProvider();
    std::string providerType = provider->getComponent()->getFullName();
    co::int32 facetIdx = returned->getFacet()->getIndex();
    co::int32 instanceId;
    
    if( ClientProxy::isLocalObject( provider ) )
    {
        instanceId = _node->publishAnonymousInstance( provider, caller );
        _marshaller.marshalReferenceType( instanceId, facetIdx, Marshaller::RefOwner::LOCAL, 
							marshalledReturn, &providerType, &_node->getPublicAddress() );
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        instanceId = providerCP->getInstanceId();
        std::string ownerAddress = providerCP->getOwnerAddress();
        
        _node->requestBeginAccess( ownerAddress, instanceId, caller );
        _marshaller.marshalReferenceType( instanceId, facetIdx, Marshaller::RefOwner::ANOTHER, 
                                      marshalledReturn, &providerType, &ownerAddress );
    }
}

void Invoker::onServiceFirstAccess( co::int32 serviceId )
{
	co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::IService* service = _object->getServiceAt( port );

    co::IInterface* itf = service->getInterface();

    co::IReflector* reflector = itf->getReflector();

	// saving for easier later access
	_openedServices[serviceId] = service;
    _openedInterfaces[serviceId] = itf;
    _openedReflectors[serviceId] = reflector;
}
    
}
    
} // namespace reef


