#include "Requestor.h"

#include "Node.h"
#include "ClientProxy.h"
#include "ClientRequestHandler.h"

#include <co/IMethod.h>
#include <co/IField.h>
#include <co/IComponent.h>

namespace reef {
namespace rpc {

Requestor::Requestor( ClientRequestHandler* handler, const std::string& localEndpoint ) : 
        _handler( handler ), _endpoint( handler->getEndpoint() ), _localEndpoint( localEndpoint )
{
    requestLease( 0 );
}
    
Requestor::~Requestor()
{
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
    
    for( ; args; args.popFirst() )
    {
        const co::Any& arg = args.getFirst();
        
        if( arg.getKind() != co::TK_INTERFACE )
            _marshaller.addValueParam( arg );
        else
            onInterfaceParam( arg.get<co::IService*>() );
    }
    
    std::string msg;
    _marshaller.getMarshalledCall( msg );
    _handler->handleAsynchRequest( msg );    
}
    

void Requestor::requestSynchCall( MemberOwner& owner, co::IMethod* method, 
                                           co::Range<co::Any const> args, co::Any& ret  )
{
    
}
    
void Requestor::requestSetField( MemberOwner& owner, co::IField* field, const co::Any arg )
{
    
}
    
void Requestor::requestGetField( MemberOwner& owner, co::IField* field, co::Any& ret )
{
    
}
    
void Requestor::requestLease( co::int32 instanceID )
{
    
}

void Requestor::requestLeaseBreak( co::int32 instanceID )
{
    
}

void Requestor::onInterfaceParam( co::IService* param )
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

    
}
}