#include "Node.h"

#include "Unmarshaller.h"
#include "Invoker.h"
#include "ClientProxy.h"

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>
#include <co/Exception.h>

#include <iostream>
#include <map>

namespace reef {
namespace rpc {


Node::Node() : _passiveLink( 0 )
{
}
    
Node::~Node()
{
    if( _passiveLink.get() )
        stop();
}
    
co::IObject* Node::newRemoteInstance( const std::string& instanceType, 
                                           const std::string& address )
{
    IActiveLink* link = _transport->connect( address );
    co::int32 instanceID = requestNewInstance( link, instanceType );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceID );
}

co::IObject* Node::findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    IActiveLink* link = _transport->connect( address );
    co::int32 instanceID = requestFindInstance( link, key );
    
    if( instanceID == 0 )
        return 0;
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceID );
}
    
void Node::start( const std::string&  boundAddress, const std::string& publicAddress )
{
    assert( !_passiveLink.get() );
    
    _passiveLink = _transport->bind( boundAddress );
    
    _myPublicAddress = publicAddress;
    
    // This first instanceID is for the Node
    _invokers.push_back( 0 );
    _remoteRefCounting.push_back( 0 );
}
    
void Node::update()
{
    assert( _passiveLink.get() );
        
	std::string msg;
    if( _passiveLink->receive( msg ) )
        dispatchMessage( msg );
}

void Node::stop()
{
    assert( _passiveLink.get() );
    
    // fill the empty holes in the invokers vector
    for( ; !_freedIds.empty(); _freedIds.pop() )
    {
        if( _freedIds.top() <= _invokers.size() )
            _invokers[_freedIds.top()] = _invokers.back();
        
        _invokers.pop_back();
    }
    
    // now delete all the invokers
    size_t size = _invokers.size();
    for( int i = 1; i < size; i++ )
    {
        delete _invokers[i];
    }
    
    _passiveLink = 0;
}

co::IObject* Node::getInstance( co::int32 instanceID )
{
    return instanceID < _invokers.size() ? _invokers[instanceID]->getObject() : 0;
}

co::int32 Node::getRemoteReferences( co::int32 instanceID )
{
    return instanceID < _remoteRefCounting.size() ? _remoteRefCounting[instanceID] : 0;
}
 
co::int32 Node::publishInstance( co::IObject* instance, const std::string& key )
{
    // Makes the instance available for remote usage as any other instance
    co::int32 instanceID = publishAnonymousInstance( instance );
    
    _publicInstances.insert( std::pair<std::string, co::int32>( key, instanceID ) );
    
    return instanceID;
}
    
void Node::unpublishInstance( const std::string& key )
{
    std::map<std::string, co::int32>::iterator it = _publicInstances.find( key );
    if( it == _publicInstances.end() )
        throw new co::Exception( "The provided key does not match any public instance" );
    
    closeRemoteReference( it->second );
    _publicInstances.erase( it );
}

co::IObject* Node::getRemoteInstance( const std::string& instanceType, co::int32 instanceID, 
                                     const std::string& ownerAddress )
{
    IActiveLink* link = _transport->connect( ownerAddress );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceID );
}

co::int32 Node::requestNewInstance( IActiveLink* link, const std::string& componentName )
{
    std::string msg;
    _marshaller.marshalNewInstance( componentName, _myPublicAddress, msg );
    link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !link->receiveReply( msg ) )
        update();
    
    co::int32 instanceID;
    _unmarshaller.unmarshalData( msg, instanceID );
    
    return instanceID;
}

co::int32 Node::requestFindInstance( IActiveLink* link, const std::string& key )
{
    std::string msg;
    _marshaller.marshalFindInstance( key, _myPublicAddress, msg );
    link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !link->receiveReply( msg ) )
        update();
    
    co::int32 instanceID;
    _unmarshaller.unmarshalData( msg, instanceID );
    
    return instanceID;
}

void Node::dispatchMessage( const std::string& msg )
{
    co::int32 destInstanceID;
    Unmarshaller::MsgType type;
    bool hasReturn;
    std::string referer;
    _unmarshaller.setMarshalledRequest( msg, type, destInstanceID, hasReturn, &referer );
    
    switch( type )
    {
        case Unmarshaller::MsgType::NEW_INST:
            onNewInstMsg();
            break;
        case Unmarshaller::MsgType::ACCESS_INST:
            onAccessInstMsg();
            break;
        case Unmarshaller::MsgType::FIND_INST:
            onFindInstMsg();
            break;
        default: // the message isn't destined to the Node. Pass to the appropriate invoker. 
            onMsgForInvoker( destInstanceID, hasReturn );
    }  
}
    
void Node::onNewInstMsg() 
{
    std::string instanceTypeName;
    _unmarshaller.unmarshalNewInstance( instanceTypeName );
    
	co::IObject* instance = co::newInstance( instanceTypeName );
    co::int32 instanceID = startRemoteRefCount( instance );
    
    // Encode the new instanceID as reply message
    std::string msg;
    _marshaller.marshalData( instanceID, msg );
    
    // reply it to the client
    _passiveLink->sendReply( msg );
}
    
void Node::onFindInstMsg() 
{
    std::string key;
    _unmarshaller.unmarshalFindInstance( key );
    
    std::map<std::string, co::int32>::iterator it = _publicInstances.find( key );
    co::int32 instanceID = 0;
    if( it != _publicInstances.end() )
    {
        instanceID = it->second;
        openRemoteReference( instanceID );
    }
    
    // Encode the instanceID (or 0 if not found) as reply message
    std::string msg;
    _marshaller.marshalData( instanceID, msg );
    
    // reply it to the client
    _passiveLink->sendReply( msg );
}
    
void Node::onAccessInstMsg()
{
    co::int32 instanceID;
    bool increment;
    
    _unmarshaller.unmarshalAccessInstance( instanceID, increment );
    
    if( increment )
        openRemoteReference( instanceID );
    else
        closeRemoteReference( instanceID );
}
  
void Node::onMsgForInvoker( co::int32 instanceID, bool hasReturn )
{
    Invoker* invoker = getInvokerFor( instanceID );
    if( !hasReturn )
    {
        invoker->onCallOrField( _unmarshaller );
        return;
    }
    
    co::Any retValue;
    retValue.set<co::int32>( 42 );
    invoker->onCallOrField( _unmarshaller, &retValue );
    
    std::string msg;
    _marshaller.marshalData( retValue, msg );
    
    _passiveLink->sendReply( msg );
}

co::int32 Node::publishAnonymousInstance( co::IObject* instance )
{
    co::int32 instanceID = getInstanceID( instance );
    if( instanceID != -1 )
        openRemoteReference( instanceID ); // TODO set referer
    else
        instanceID = startRemoteRefCount( instance );
    
    return instanceID;
}
    
void Node::requestBeginAccess( const std::string& address, co::int32 instanceID,
                                 const std::string& referer )
{
    IActiveLink* link = _transport->connect( address );
    
    std::string msg;
    _marshaller.marshalAccessInstance( instanceID, true, referer, msg );
    link->send( msg );
}
  
void Node::requestEndAccess( IActiveLink* link, co::int32 instanceID, const std::string& referer )
{    
    std::string msg;
    _marshaller.marshalAccessInstance( instanceID, false, referer, msg );
    link->send( msg );
}
    
void Node::openRemoteReference( co::int32 instanceID )
{
    _remoteRefCounting[instanceID]++;
}
    
void Node::closeRemoteReference( co::int32 instanceID )
{
    if( --_remoteRefCounting[instanceID] < 1 )
        releaseInstance( instanceID );
    
}
    
co::int32 Node::getInstanceID( const co::IObject* instance )
{
    VirtualAddresses::iterator it = _vas.find( instance );
    if( it != _vas.end() )
        return (*it).second;
    
    return -1;
}

Invoker* Node::getInvokerFor( co::int32 instanceID )
{ 
    return _invokers[instanceID]; 
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
    
    _invokers[newInvokerId] = new Invoker( this, instance );
    _remoteRefCounting[newInvokerId] = 1; // TODO set referer
    
    return newInvokerId;
}

void Node::releaseInstance( co::int32 instanceID )
{
    co::IObject* instance = _invokers[instanceID]->getObject();
    VirtualAddresses::iterator it = _vas.find( instance );
    _vas.erase( it );
    
    delete _invokers[instanceID];
    _freedIds.push( instanceID );
    
}

reef::rpc::ITransport* Node::getTransportService()
{
    return _transport;
}

void Node::setTransportService( reef::rpc::ITransport* transport )
{
    _transport = transport;
}
    
CORAL_EXPORT_COMPONENT( Node, Node );
    
}
    
} // namespace reef