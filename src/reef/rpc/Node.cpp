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

/*!
 \brief Internal class that maps a Client Node to all the local instance's Ids. 
 
 This mapping is purely informative (for usage in algorithms that need to know a reference owner). 
 The actual reference counting that manages the instances' lifecycle happens elsewhere.
 */
class Client
{
public:
    Client();
    
    /*
        Adds the Id of a local instance to the list of Ids that this client refers. This method is
        merely add the ID to the list for usage in algorithms that need to consult the owner of a
        reference.
     */
    inline void addReferredId( co::int32 instanceId )
    {
        _ids.insert( instanceId );
    }
    
    /*
        Analogous to addRefferedID but removes the id.
    */
    inline void removeReferredId( co::int32 instanceId )
    {
        _ids.erase( instanceId );
    }
    
    /*
        Searches this client references for \param instanceId , returns true if found, false if not.
    */
    inline bool searchReference( co::int32 instanceId )
    {
        _it = _ids.find( instanceId );
        return _it != _ids.end() ? true : false;
    }
    
    inline bool isEmpty()
    {
        return _ids.empty();
    }
    
private:
    std::set<co::int32> _ids;
    std::set<co::int32>::iterator _it;
};

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
    co::int32 instanceId = requestNewInstance( link, instanceType );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceId );
}

co::IObject* Node::findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    IActiveLink* link = _transport->connect( address );
    co::int32 instanceId = requestFindInstance( link, key );
    
    if( instanceId == 0 )
        return 0;
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceId );
}
    
void Node::start( const std::string&  boundAddress, const std::string& publicAddress )
{
    assert( !_passiveLink.get() );
    
    _passiveLink = _transport->bind( boundAddress );
    
    _myPublicAddress = publicAddress;
    
    // This first instanceId is for the Node
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
    co::int32 instanceId = publishAnonymousInstance( instance );
    
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

co::IObject* Node::getRemoteInstance( const std::string& instanceType, co::int32 instanceId, 
                                     const std::string& ownerAddress )
{
    IActiveLink* link = _transport->connect( ownerAddress );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return ClientProxy::getOrCreateClientProxy( this, component, link, instanceId );
}

co::int32 Node::requestNewInstance( IActiveLink* link, const std::string& componentName )
{
    std::string msg;
    _marshaller.marshalNewInstance( componentName, _myPublicAddress, msg );
    link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !link->receiveReply( msg ) )
        update();
    
    co::int32 instanceId;
    _unmarshaller.unmarshalData( msg, instanceId );
    
    return instanceId;
}

co::int32 Node::requestFindInstance( IActiveLink* link, const std::string& key )
{
    std::string msg;
    _marshaller.marshalFindInstance( key, _myPublicAddress, msg );
    link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !link->receiveReply( msg ) )
        update();
    
    co::int32 instanceId;
    _unmarshaller.unmarshalData( msg, instanceId );
    
    return instanceId;
}

void Node::dispatchMessage( const std::string& msg )
{
    co::int32 destInstanceId;
    Unmarshaller::MsgType type;
    bool hasReturn;
    _unmarshaller.setMarshalledRequest( msg, type, destInstanceId, hasReturn );
    
    switch( type )
    {
        case Unmarshaller::NEW_INST:
            onNewInstMsg();
            break;
        case Unmarshaller::ACCESS_INST:
            onAccessInstMsg();
            break;
        case Unmarshaller::FIND_INST:
            onFindInstMsg();
            break;
        default: // the message isn't destined to the Node. Pass to the appropriate invoker. 
            onMsgForInvoker( destinstanceId, hasReturn );
    }  
}
    
void Node::onNewInstMsg() 
{
    std::string instanceTypeName;
    std::string referer;
    _unmarshaller.unmarshalNewInstance( instanceTypeName, referer );
    
	co::IObject* instance = co::newInstance( instanceTypeName );
    co::int32 instanceId = startRemoteRefCount( instance );
    
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
    _unmarshaller.unmarshalFindInstance( key, referer );
    
    std::map<std::string, co::int32>::iterator it = _publicInstances.find( key );
    co::int32 instanceId = 0;
    if( it != _publicInstances.end() )
    {
        instanceId = it->second;
        openRemoteReference( instanceId );
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
    
    _unmarshaller.unmarshalAccessInstance( instanceId, increment, referer );
    
    if( increment )
        openRemoteReference( instanceId );
    else
        closeRemoteReference( instanceId );
}
  
void Node::onMsgForInvoker( co::int32 instanceId, bool hasReturn )
{
    Invoker* invoker = getInvokerFor( instanceId );
    
    std::string returned;
    invoker->invoke( _unmarshaller, hasReturn, returned );
    
    if( hasReturn )
        _passiveLink->sendReply( returned );
}

co::int32 Node::publishAnonymousInstance( co::IObject* instance )
{
    co::int32 instanceId = getinstanceId( instance );
    if( instanceId != -1 )
        openRemoteReference( instanceId ); // TODO set referer
    else
        instanceId = startRemoteRefCount( instance );
    
    return instanceId;
}
    
void Node::requestBeginAccess( const std::string& address, co::int32 instanceId,
                                 const std::string& referer )
{
    IActiveLink* link = _transport->connect( address );
    
    std::string msg;
    _marshaller.marshalAccessInstance( instanceId, true, referer, msg );
    link->send( msg );
}
  
void Node::requestEndAccess( IActiveLink* link, co::int32 instanceId, const std::string& referer )
{    
    std::string msg;
    _marshaller.marshalAccessInstance( instanceId, false, referer, msg );
    link->send( msg );
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
    
    _invokers[newInvokerId] = new Invoker( this, instance );
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
    
Client* getReferer( const std::string& ip )
{
    std::map<std::string, Client*>::iterator it = _referers.find( ip );
    return  it == _referers.end() ? 0: *it;
}

    
CORAL_EXPORT_COMPONENT( Node, Node );
    
}
    
} // namespace reef