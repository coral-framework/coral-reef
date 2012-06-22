#include "Node.h"

#include "Invoker.h"
#include "Requestor.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "ServerRequestHandler.h"

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>
#include <co/Exception.h>

#include <iostream>
#include <cassert>
#include <map>
#include <set>

namespace reef {
namespace rpc {
    
Node::Node() : _srh( 0 )
{
}
    
Node::~Node()
{
    assert( _publicEndpoint.empty() ); // Node needs to be stopped
}
    
co::IObject* Node::newRemoteInstance( const std::string& instanceType, 
                                           const std::string& address )
{
    Requestor* req = _requestorMan->getOrCreateRequestor( address );
    return req->requestNewInstance( instanceType );
}

co::IObject* Node::findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    Requestor* req = _requestorMan->getOrCreateRequestor( address );
    return req->requestPublicInstance( key, instanceType );
}
    
void Node::start( const std::string&  boundAddress, const std::string& publicAddress )
{
    _publicEndpoint = publicAddress;
    
    _srh = new ServerRequestHandler( _transport->bind( boundAddress ) );

    _instanceMan = new InstanceManager();
 
    _requestorMan = new RequestorManager( this, _instanceMan, _transport, _publicEndpoint );
    
    _invoker = new Invoker( this, _instanceMan, _srh, _requestorMan );
}
    
void Node::update()
{
    assert( _srh );
        
	_srh->react();
}

void Node::stop()
{
    assert( !_publicEndpoint.empty() );
    
    delete _invoker; _invoker = 0;
    delete _requestorMan; _requestorMan = 0;
    delete _instanceMan; _instanceMan = 0;
    delete _srh; _srh = 0;
}

co::IObject* Node::getInstance( co::int32 instanceId )
{
    return instanceId < _invokers.size() ? _invokers[instanceId]->getObject() : 0;
}

co::int32 Node::getRemoteReferences( co::int32 instanceId )
{
    return instanceId < _remoteRefCounting.size() ? _remoteRefCounting[instanceId] : 0;
}
 
co::int32 Node::publishInstance( co::IObject* instance, const std::string& key )
{
    // Makes the instance available for remote usage as any other instance
    co::int32 instanceId = publishAnonymousInstance( instance, "self" );
    
    _publicInstances.insert( std::pair<std::string, co::int32>( key, instanceId ) );
    
    return instanceId;
}
    
void Node::unpublishInstance( const std::string& key )
{
    std::map<std::string, co::int32>::iterator it = _publicInstances.find( key );
    if( it == _publicInstances.end() )
        throw new co::Exception( "The provided key does not match any public instance" );
    
    closeRemoteReference( it->second );
    _publicInstances.erase( it );
}
    
void Node::dispatchMessage( const std::string& msg )
{
    co::int32 destInstanceId;
    Demarshaller::MsgType type;
    bool hasReturn;
    _demarshaller.setMarshalledRequest( msg, type, destInstanceId, hasReturn );
    
    switch( type )
    {
        case Demarshaller::NEW_INST:
            onNewInstMsg();
            break;
        case Demarshaller::ACCESS_INST:
            onAccessInstMsg();
            break;
        case Demarshaller::FIND_INST:
            onFindInstMsg();
            break;
        default: // the message isn't destined to the Node. Pass to the appropriate invoker. 
            onMsgForInvoker( destInstanceId, hasReturn );
    }  
}
    
void Node::onNewInstMsg() 
{
    std::string instanceTypeName;
    std::string referer;
    _demarshaller.demarshalNewInstance( instanceTypeName, referer );
    
	co::IObject* instance = co::newInstance( instanceTypeName );
    co::int32 instanceId = startRemoteRefCount( instance );
    
    Client* client = tryAddReferer( referer );
    client->addReferredId( instanceId );
    
    // Encode the new instanceId as reply message
    std::string msg;
    _marshaller.marshalData( instanceId, msg );
    
    // reply it to the client
    _passiveLink->sendReply( msg );
}
    
void Node::onFindInstMsg() 
{
    std::string key;
    std::string referer;
    _demarshaller.demarshalFindInstance( key, referer );
    
    std::map<std::string, co::int32>::iterator it = _publicInstances.find( key );
    co::int32 instanceId = 0;
    if( it != _publicInstances.end() )
    {
        instanceId = it->second;
        openRemoteReference( instanceId );
        Client* client = tryAddReferer( referer );
        client->addReferredId( instanceId );
    }
    
    // Encode the instanceId (or 0 if not found) as reply message
    std::string msg;
    _marshaller.marshalData( instanceId, msg );
    
    // reply it to the client
    _passiveLink->sendReply( msg );
}
    
void Node::onAccessInstMsg()
{
    co::int32 instanceId;
    bool increment;
    std::string referer;
    
    // REMOTINGERROR: if no instance found with the id
    _demarshaller.demarshalAccessInstance( instanceId, increment, referer );
    
    if( increment )
    {
        Client* client = tryAddReferer( referer );
        if( client->addReferredId( instanceId ) ) // If there isn't already a reference
            openRemoteReference( instanceId );
    }
    else
    {
        tryRemoveReferer( referer, instanceId );
                
        closeRemoteReference( instanceId );
    }
}
  
void Node::onMsgForInvoker( co::int32 instanceId, bool hasReturn )
{
    Invoker* invoker = getInvokerFor( instanceId );
    
    std::string returned;
    invoker->invoke( _demarshaller, hasReturn, returned );
    
    if( hasReturn )
        _passiveLink->sendReply( returned );
}

co::int32 Node::publishAnonymousInstance( co::IObject* instance, const std::string& referer )
{
    co::int32 instanceId = getinstanceId( instance );
    if( instanceId != -1 )
        openRemoteReference( instanceId );
    else
        instanceId = startRemoteRefCount( instance );
    
    if( referer != "self" )
    {
        Client* client = tryAddReferer( referer );
        client->addReferredId( instanceId );
    }
    
    return instanceId;
}

void Node::openRemoteReference( co::int32 instanceId )
{
    _remoteRefCounting[instanceId]++;
}
    
void Node::closeRemoteReference( co::int32 instanceId )
{
    if( --_remoteRefCounting[instanceId] < 1 )
        releaseInstance( instanceId );
    
}
    
co::int32 Node::getinstanceId( const co::IObject* instance )
{
    VirtualAddresses::iterator it = _vas.find( instance );
    if( it != _vas.end() )
        return (*it).second;
    
    return -1;
}

Invoker* Node::getInvokerFor( co::int32 instanceId )
{ 
    return _invokers[instanceId]; 
}
    
co::int32 Node::newVirtualAddress()
{
    if( !_freedIds.empty() )
    {
        co::int32 newInvokerId = _freedIds.top();
        _freedIds.pop();
        return newInvokerId;
    }
    else
    {
        _invokers.push_back( 0 );
        _remoteRefCounting.push_back( 0 );
        return _invokers.size() - 1;
    }
    
}
   
co::int32 Node::startRemoteRefCount( co::IObject* instance )
{
    co::int32 newInvokerId = newVirtualAddress();
    _vas.insert( objToAddress( instance, newInvokerId ) );
    
    _invokers[newInvokerId] = new Invoker( this, _requestorMan, instance );
    _remoteRefCounting[newInvokerId] = 1; // TODO set referer
    
    return newInvokerId;
}

void Node::releaseInstance( co::int32 instanceId )
{
    co::IObject* instance = _invokers[instanceId]->getObject();
    VirtualAddresses::iterator it = _vas.find( instance );
    _vas.erase( it );
    
    delete _invokers[instanceId];
    _freedIds.push( instanceId );
    
}

reef::rpc::ITransport* Node::getTransportService()
{
    return _transport;
}

void Node::setTransportService( reef::rpc::ITransport* transport )
{
    _transport = transport;
}
    
Client* Node::findReferer( const std::string& ip )
{
    std::map<std::string, Client*>::iterator it = _referers.find( ip );
    return  it == _referers.end() ? 0: it->second;
}

Client* Node::tryAddReferer( const std::string& ip )
{
    Client* client = findReferer( ip );

    if( client )
        return client;
    
    client = new Client();
    _referers.insert( std::pair<std::string, Client*>( ip, client ) );
    return client;
}
  
bool Node::tryRemoveReferer( const std::string& ip, co::int32 instanceId )
{
    std::map<std::string, Client*>::iterator it = _referers.find( ip );
    assert( it != _referers.end() ); //REMOTINGERROR There is no Client associated with ip
    
    Client* client = it->second;
    if( !client->removeReferredId( instanceId ) )
    {
        assert( false ); //REMOTINGERROR There is no reference to the Id
    }
    
    if( !client->isEmpty() )
        return false;

    _referers.erase( it );
    delete client;
    return true;
}
    
CORAL_EXPORT_COMPONENT( Node, Node );
    
}
    
} // namespace reef