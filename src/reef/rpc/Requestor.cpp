#include "Requestor.h"

#include "Node.h"
#include "ClientProxy.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "ClientRequestHandler.h"

#include <co/IMethod.h>
#include <co/IField.h>
#include <co/IComponent.h>

namespace reef {
namespace rpc {

Requestor::Requestor( RequestorManager* manager, ClientRequestHandler* handler, 
                    const std::string& localEndpoint ) :  _manager( manager ),  _handler( handler ), 
                    _endpoint( handler->getEndpoint() ), _localEndpoint( localEndpoint ), 
                    _node( manager->getNode() ), _instanceMan( manager->getInstanceManager() )
{
}
    
Requestor::~Requestor()
{
    delete _handler;
}

co::IObject* Requestor::requestNewInstance( const std::string& componentName )
{
    std::string msg;
    _marshaller.marshalNewInstance( componentName, _localEndpoint, msg );
 
    _handler->handleSynchRequest( msg, msg );
    
    co::int32 instanceID;
    _demarshaller.demarshalData( msg, instanceID );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( componentName ) );
    
    ClientProxy* cp = new ClientProxy( this, component, instanceID );
    _proxies.insert( std::pair<co::int32, ClientProxy*>( instanceID, cp ) );
    
    return cp;
}

co::IObject* Requestor::requestPublicInstance( const std::string& key, 
                                              const std::string& componentName )
{
    std::string msg;
    _marshaller.marshalFindInstance( key, _localEndpoint, msg );
    
    _handler->handleSynchRequest( msg, msg );
    
    co::int32 instanceID;
    _demarshaller.demarshalData( msg, instanceID );
    
    // First, search if there isnt already a cp to the instanceID
    std::map<co::int32, ClientProxy*>::iterator it = _proxies.find( instanceID );
    if( it != _proxies.end() )
        return it->second;
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( componentName ) );
    
    ClientProxy* cp = new ClientProxy( this, component, instanceID );
    _proxies.insert( std::pair<co::int32, ClientProxy*>( instanceID, cp ) );

    return cp;
}

void Requestor::requestAsynchCall( MemberOwner& owner, co::IMethod* method,  
                                            co::Range<co::Any const> args )
{
    _marshaller.beginCallMarshalling( owner.instanceID, owner.facetID, method->getIndex(), 
                                     owner.inheritanceDepth, false, _localEndpoint );
    
    marshalParameters( method, args );
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _handler->handleAsynchRequest( msg );    
}
    

void Requestor::requestSynchCall( MemberOwner& owner, co::IMethod* method, 
                                           co::Range<co::Any const> args, co::Any& ret  )
{
    _marshaller.beginCallMarshalling( owner.instanceID, owner.facetID, method->getIndex(), 
                                     owner.inheritanceDepth, true, _localEndpoint );
    
    marshalParameters( method, args );
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _handler->handleSynchRequest( msg, msg );
    
    demarshalReturn( msg, method->getReturnType(), ret );
}
    
void Requestor::requestSetField( MemberOwner& owner, co::IField* field, const co::Any arg )
{
    _marshaller.beginCallMarshalling( owner.instanceID, owner.facetID, field->getIndex(), 
                                     owner.inheritanceDepth, false, _localEndpoint );
    
    if( arg.getKind() != co::TK_INTERFACE )
        _marshaller.addValueParam( arg );
    else
        marshalProviderInfo( arg.get<co::IService*>() );
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _handler->handleAsynchRequest( msg );
}
    
void Requestor::requestGetField( MemberOwner& owner, co::IField* field, co::Any& ret )
{
    _marshaller.beginCallMarshalling( owner.instanceID, owner.facetID, field->getIndex(), 
                                     owner.inheritanceDepth, true, _localEndpoint );
    
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _handler->handleSynchRequest( msg, msg );
    
    demarshalReturn( msg, field->getType(), ret );
}
    
void Requestor::requestLease( co::int32 instanceID, std::string lessee )
{
    Marshaller marshaller; // cannot use the member instance (conflicts with other marshalling)
    std::string msg;
    marshaller.marshalAccessInstance( instanceID, true, lessee, msg );
    _handler->handleAsynchRequest( msg );
}

void Requestor::requestCancelLease( co::int32 instanceID )
{
    std::string msg;
    _marshaller.marshalAccessInstance( instanceID, false, _localEndpoint, msg );
    _handler->handleAsynchRequest( msg );
    
    size_t result = _proxies.erase( instanceID );
    assert( result );
    
    if( _proxies.size() <= 0 )
        lastProxyRemoved();
}
    
ClientProxy* Requestor::getOrCreateProxy( co::int32 instanceID, const std::string& componentName )
{
    std::map<co::int32, ClientProxy*>::iterator it = _proxies.find( instanceID );
    if( it != _proxies.end() )
        return it->second;

    co::IComponent* component = co::cast<co::IComponent>( co::getType( componentName ) );
    ClientProxy* cp = new ClientProxy( this, component, instanceID );
    _proxies.insert( std::pair<co::int32, ClientProxy*>( instanceID, cp ) );
    
    return cp;
}
    
void Requestor::marshalParameters( co::IMethod* method, co::Range<co::Any const> args )
{
    for( ; args; args.popFirst() )
    {
        const co::Any& arg = args.getFirst();
        
        if( arg.getKind() != co::TK_INTERFACE )
            _marshaller.addValueParam( arg );
        else
            marshalProviderInfo( arg.get<co::IService*>() );
    }
}

void Requestor::marshalProviderInfo( co::IService* param )
{
    co::IObject* provider = param->getProvider();
    std::string providerType = provider->getComponent()->getFullName();
    co::int32 facetIdx = param->getFacet()->getIndex();
    co::int32 instanceId;
    
    if( !ClientProxy::isClientProxy( provider ) )
    {
        
        instanceId = _instanceMan->addInstance( provider, _endpoint );
        _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::LOCAL, &providerType, 
                                      &_node->getPublicEndpoint() );
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        instanceId = providerCP->getInstanceId();
        Requestor* requestor = providerCP->getRequestor();
        
        if( requestor == this ) // Receiver
        {
            _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::RECEIVER );
        }
        else
        {
            requestor->requestLease( instanceId, _endpoint );
            std::string othersEndpoint( requestor->getEndpoint() );
            _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::ANOTHER, &providerType,
                                          &othersEndpoint );
        }
    }
}

void Requestor::demarshalReturn( const std::string& data, co::IType* returnedType, co::Any& ret )
{
    if( returnedType->getKind() != co::TK_INTERFACE )
    {
        _demarshaller.demarshalValue( data, returnedType, ret );
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
            Requestor* req = _manager->getOrCreateRequestor( ownerAddress );
            instance = req->getOrCreateProxy( instanceId, instanceType );
            break;
    }
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    co::IService* service = instance->getServiceAt( port );
    ret.set<co::IService*>( service );
}

void Requestor::lastProxyRemoved()
{
	_manager->onRequestorDestroyed( _handler->getEndpoint() );
	delete this;
}
    
}
}