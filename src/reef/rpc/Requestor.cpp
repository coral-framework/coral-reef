#include "Requestor.h"

#include "Node.h"
#include "ClientProxy.h"
#include "RequestorCache.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IActiveLink.h>
#include <co/IMethod.h>
#include <co?IField.h>
#include <co/IComponent.h>

namespace reef {
namespace rpc {

Requestor::Requestor( Node* node, IActiveLink* link, const std::string& localEndpoint, 
        RequestorCache* manager ) : _node( node ), _link( link ), _endpoint( link->getAddress() ),
        _localEndpoint( localEndpoint ), _manager( manager )
{
}
    
Requestor::~Requestor()
{
    _manager->onRequestorDestroyed( _endpoint );
}

co::IObject* Requestor::requestNewInstance( const std::string& componentName )
{
    std::string msg;
    _marshaller.marshalNewInstance( componentName, _localEndpoint, msg );
    _link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !_link->receiveReply( msg ) )
        _node->update();
    
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
    _link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !_link->receiveReply( msg ) )
        _node->update();
    
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

void Requestor::requestAsynchInvocation( co::int32 instanceID, co::int32 dynFacetId, 
                co::int32 inheritanceDepth, co::IMethod* method, co::Range<co::Any const> args )
{
    _marshaller.beginCallMarshalling( instanceID, dynFacetId, method->getIndex(), inheritanceDepth, returnType,
                                     _node->getPublicAddress() );
    
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
    _link->send( msg );
    
	if( returnType )
    {
        awaitReplyUpdating( msg );
        demarshalReturn( msg, method->getReturnType(), _resultBuffer );
    }

    
}
    
void Requestor::requestAsynchInvocation( co::int32 dynFacetId, co::int32 inheritanceDepth,
                                            co::IField* field, const co::Any arg )
{
    
}

void Requestor::requestSynchInvocation( co::int32 dynFacetId, co::int32 inheritanceDepth,
                                co::IMethod* method, co::Range<co::Any const> args, co::Any& ret  )
{
    
}
    
void Requestor::requestSynchInvocation( co::int32 dynFacetId, co::int32 inheritanceDepth,
                                           co::IField* field, co::Any& ret )
{
    
}
    
void Requestor::requestLease( co::int32 instanceID )
{
    
}

void Requestor::requestLeaseBreak( co::int32 instanceID )
{
    
}

}
}