#include "Requestor.h"

#include "Node.h"
#include "ClientProxy.h"
#include "InstanceManager.h"
#include "InstanceContainer.h"
#include "RequestorManager.h"
#include "ClientRequestHandler.h"

#include <reef/rpc/RemotingException.h>

#include <co/Log.h>
#include <co/IReflector.h>
#include <co/IComponent.h>

namespace reef {
namespace rpc {

Requestor::Requestor( RequestorManager* manager, ClientRequestHandler* handler, 
            const std::string& publicEndpoint ) : _connected( true ), _manager( manager ), 
                _handler( handler ), _instanceMan( manager->getInstanceManager() ),
                     _endpoint( handler->getEndpoint() ), _publicEndpoint( publicEndpoint )
{
}
    
void Requestor::disconnect()
{
    _connected = false;
}
    
Requestor::~Requestor()
{
    if( _connected )
        _manager->onRequestorDestroyed( _handler->getEndpoint() );
    
    delete _handler;
}

co::IObject* Requestor::requestNewInstance( const std::string& componentName )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    std::string msg;
    _marshaller.marshalNew( _publicEndpoint, componentName, msg );
 
    _handler->handleSynchRequest( msg, msg );
    
    _demarshaller.demarshal( msg );
    co::int32 instanceID = _demarshaller.getIntReturn();
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( componentName ) );
    
    ClientProxy* cp = new ClientProxy( this, component, instanceID );
    _proxies.insert( std::pair<co::int32, ClientProxy*>( instanceID, cp ) );
    
    return cp;
}

co::IObject* Requestor::requestPublicInstance( const std::string& key, 
                                              const std::string& componentName )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    std::string msg;
    _marshaller.marshalLookup( _publicEndpoint, key, msg );
    
    _handler->handleSynchRequest( msg, msg );
    
    _demarshaller.demarshal( msg );
    co::int32 instanceID = _demarshaller.getIntReturn();
    if( instanceID == -1 )
        return 0;
    
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
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    InvocationDetails details( owner.instanceID, owner.facetID, method->getIndex(), 
                              owner.inheritanceDepth, false );
    ParameterPusher& pusher = _marshaller.beginInvocation( _publicEndpoint, details );
    
    pushParameters( method, args, pusher );
    
    std::string msg;
    _marshaller.marshalInvocation( msg );
    _handler->handleAsynchRequest( msg );    
}

void Requestor::requestSynchCall( MemberOwner& owner, co::IMethod* method, 
                                           co::Range<co::Any const> args, co::Any& ret  )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    InvocationDetails details( owner.instanceID, owner.facetID, method->getIndex(), 
                              owner.inheritanceDepth, true );
    ParameterPusher& pusher = _marshaller.beginInvocation( _publicEndpoint, details );
    
    pushParameters( method, args, pusher );
    
    std::string msg;
    _marshaller.marshalInvocation( msg );

    _handler->handleSynchRequest( msg, msg );
    
    getReturn( msg, method->getReturnType(), ret );
}
    
void Requestor::requestSetField( MemberOwner& owner, co::IField* field, const co::Any arg )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    InvocationDetails details( owner.instanceID, owner.facetID, field->getIndex(), 
                              owner.inheritanceDepth, false );
    ParameterPusher& pusher = _marshaller.beginInvocation( _publicEndpoint, details );
    
    if( arg.getKind() != co::TK_INTERFACE )
    {
        pusher.pushValue( arg );
    }
    else
    {
        ReferenceType refType;
        getProviderInfo( arg.get<co::IService*>(), refType );
        pusher.pushReference( refType );
    }
    
    std::string msg;
    _marshaller.marshalInvocation( msg );
    _handler->handleAsynchRequest( msg );
}
    
void Requestor::requestGetField( MemberOwner& owner, co::IField* field, co::Any& ret )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    InvocationDetails details( owner.instanceID, owner.facetID, field->getIndex(), 
                              owner.inheritanceDepth, true );
    _marshaller.beginInvocation( _publicEndpoint, details );
    
    std::string msg;
    _marshaller.marshalInvocation( msg );
    _handler->handleSynchRequest( msg, msg );
    
    getReturn( msg, field->getType(), ret );
}
    
void Requestor::requestLease( co::int32 instanceID, std::string lessee )
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    Marshaller marshaller; // cannot use the member instance (conflicts with other marshalling)
    std::string msg;
    marshaller.marshalLease( lessee, instanceID, msg );
    _handler->handleAsynchRequest( msg );
}

void Requestor::requestCancelLease( co::int32 instanceID )
{
    if( !_connected )
    {
        CORAL_LOG( WARNING ) << "Proxy request a lease cancelling with node stopped. Ideally the proxies should  be deleted before stopping the node.";
        return;
    }
        
    std::string msg;
    _marshaller.marshalCancelLease( _publicEndpoint, instanceID, msg );
    _handler->handleAsynchRequest( msg );
    
    size_t result = _proxies.erase( instanceID );
    assert( result );
}
    
void Requestor::requestBarrierUp()
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");
    
    std::string msg;
    _marshaller.marshalBarrierUp( _publicEndpoint, msg );
    _handler->handleAsynchRequest( msg );
}
  
void Requestor::requestBarrierHit()
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");

    std::string msg;
    _marshaller.marshalBarrierHit( _publicEndpoint, msg );
    _handler->handleAsynchRequest( msg );
}
    
void Requestor::requestBarrierDown()
{
    if( !_connected )
        CORAL_THROW( RemotingException, "Trying to request with the node stopped");

    std::string msg;
    _marshaller.marshalBarrierDown( _publicEndpoint, msg );
    _handler->handleAsynchRequest( msg );
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
    
void Requestor::pushParameters( co::IMethod* method, co::Range<co::Any const> args, 
                                   ParameterPusher& pusher )
{
    for( ; args; args.popFirst() )
    {
        const co::Any& arg = args.getFirst();
        
        if( arg.getKind() != co::TK_INTERFACE )
        {
            pusher.pushValue( arg );
        }
        else
        {
            ReferenceType refType;
            getProviderInfo( arg.get<co::IService*>(), refType );
            pusher.pushReference( refType );
        }
    }
}

void Requestor::getProviderInfo( co::IService* param, ReferenceType& refType )
{
    co::IObject* provider = param->getProvider();
    
    refType.instanceType = provider->getComponent()->getFullName();

    refType.facetIdx = param->getFacet()->getIndex();
    
    if( !ClientProxy::isClientProxy( provider ) )
    {
        refType.instanceID = _instanceMan->addInstance( provider, _endpoint );
        refType.owner = OWNER_SENDER;
        refType.ownerEndpoint = _publicEndpoint;
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        refType.instanceID = providerCP->getInstanceId();
        
        Requestor* requestor = providerCP->getRequestor();
        
        if( requestor == this ) // Receiver
        {
            refType.owner = OWNER_RECEIVER;
            refType.ownerEndpoint = "";
        }
        else
        {
            CORAL_DLOG( INFO ) << "Passing a parameter that is a reference to a remote instance in : "
                        << requestor->getEndpoint() << " requesting a lease now.";
            
            requestor->requestLease( refType.instanceID, _endpoint );
            refType.owner = OWNER_ANOTHER;
            refType.ownerEndpoint = requestor->getEndpoint();
        }
    }
}

void Requestor::getReturn( const std::string& data, co::IType* returnedType, co::Any& ret )
{
    MessageType type = _demarshaller.demarshal( data );
    
    if( type == EXCEPTION )
    {
        std::string exTypeName;
        std::string what;
        ExceptionType exType = _demarshaller.getException( exTypeName, what );
        if( exType == EX_REMOTING )
        {
            // TODO try to resolve the problem if possible
            //if not resolved then:
            co::getType( exTypeName )->getReflector()->raise( what );
        }
        else
        {
            co::getType( exTypeName )->getReflector()->raise( what );
        }
    }
    
    
    if( returnedType->getKind() != co::TK_INTERFACE )
    {
        _demarshaller.getValueTypeReturn( returnedType, ret );
        return;
    }
    
    ReferenceType refType;
    
    _demarshaller.getRefTypeReturn( refType );
    co::IObject* instance;
    switch( refType.owner )
    {
        case OWNER_RECEIVER:
        {
            instance = _instanceMan->getInstance( refType.instanceID )->getInstance();
            if( !instance )
                CORAL_THROW( reef::rpc::RemotingException, "Server returned an invalid reference to a local instance" );
            break;
        }
        case OWNER_SENDER:
        case OWNER_ANOTHER:
            Requestor* req = _manager->getOrCreateRequestor( refType.ownerEndpoint );
            instance = req->getOrCreateProxy( refType.instanceID, refType.instanceType );
            break;
    }
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[refType.facetIdx];
    co::IService* service = instance->getServiceAt( port );
    ret.set<co::IService*>( service );
}
    
}
}