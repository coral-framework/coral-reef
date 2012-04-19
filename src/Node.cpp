#include "Node.h"

#include "Decoder.h"
#include "Servant.h"
#include "RemoteObject.h"

#include <reef/IActiveLink.h>
#include <reef/IPassiveLink.h>
#include <co/Exception.h>

#include <iostream>
#include <map>

namespace reef {

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
    
    return RemoteObject::getOrCreateRemoteObject( this, component, link, instanceID );
}

void Node::start( const std::string&  boundAddress, const std::string& publicAddress )
{
    assert( !_passiveLink.get() );
    
    _passiveLink = _transport->bind( boundAddress );
    
    _myPublicAddress = publicAddress;
    
    // This first instanceID is for the Node
    _servants.push_back( 0 );
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
    
    // fill the empty holes in the servants vector
    for( ; !_freedIds.empty(); _freedIds.pop() )
    {
        if( _freedIds.top() != _servants.size() )
            _servants[_freedIds.top()] = _servants.back();
        
        _servants.pop_back();
    }
    
    // now delete all the servants
    size_t size = _servants.size();
    for( int i = 1; i < size; i++ )
    {
        delete _servants[i];
    }
    
    _passiveLink = 0;
}

co::IObject* Node::getRemoteInstance( const std::string& instanceType, co::int32 instanceID, 
                                     const std::string& ownerAddress )
{
    IActiveLink* link = _transport->connect( ownerAddress );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return RemoteObject::getOrCreateRemoteObject( this, component, link, instanceID );
}

co::int32 Node::requestNewInstance( IActiveLink* link, const std::string& componentName )
{
    std::string msg;
    _encoder.encodeNewInstMsg( componentName, msg );
    link->send( msg );
    
    // The Wait for the reply still keeps updating the server
    while( !link->receiveReply( msg ) )
        update();
    
    co::int32 instanceID;
    _decoder.decodeData( msg, instanceID );
    
    return instanceID;
}

void Node::dispatchMessage( const std::string& msg )
{
    co::int32 destInstanceID;
    Decoder::MsgType type;
    bool hasReturn;
    _decoder.setMsgForDecoding( msg, type, destInstanceID, hasReturn );
    
    switch( type )
    {
        case Decoder::MsgType::NEW_INST:
            onNewInstMsg();
            break;
        case Decoder::MsgType::ACCESS_INST:
            onAccessInstMsg();
            break;
        default: // the message isn't destined to the Node. Pass to the appropriate servant. 
            onMsgForServant( destInstanceID, hasReturn );
    }  
}
    
void Node::onNewInstMsg() 
{
    std::string instanceTypeName;
    _decoder.decodeNewInstMsg( instanceTypeName );
    
	co::IObject* instance = co::newInstance( instanceTypeName );
    co::int32 instanceID = startRemoteRefCount( instance );
    
    // Encode the new instanceID as reply message
    std::string msg;
    _encoder.encodeData( instanceID, msg );
    
    // reply it to the client
    _passiveLink->sendReply( msg );
}
    
void Node::onAccessInstMsg()
{
    std::string refererIP;
    co::int32 instanceID;
    bool increment;            
    _decoder.decodeAccessInstMsg( refererIP, instanceID, increment );
    openRemoteReference( instanceID );    
}
  
void Node::onMsgForServant( co::int32 instanceID, bool hasReturn )
{
    Servant* servant = getServantFor( instanceID );
    if( !hasReturn )
    {
        servant->onCallOrField( _decoder );
        return;
    }
    
    co::Any retValue;
    retValue.set<co::int32>( 42 );
    servant->onCallOrField( _decoder, &retValue );
    
    std::string msg;
    _encoder.encodeData( retValue, msg );
    
    _passiveLink->sendReply( msg );
}

co::int32 Node::publishInstance( co::IObject* instance )
{
    co::int32 instanceID = getInstanceID( instance );
    if( instanceID != -1 )
        openRemoteReference( instanceID ); // TODO set referer
    else
        instanceID = startRemoteRefCount( instance );
    
    return instanceID;
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

Servant* Node::getServantFor( co::int32 instanceID )
{ 
    return _servants[instanceID]; 
}
    
co::int32 Node::newVirtualAddress()
{
    if( !_freedIds.empty() )
    {
        co::int32 newServantId = _freedIds.top();
        _freedIds.pop();
        return newServantId;
    }
    else
    {
        _servants.push_back( 0 );
        _remoteRefCounting.push_back( 0 );
        return _servants.size() - 1;
    }
    
}
   
co::int32 Node::startRemoteRefCount( co::IObject* instance )
{
    co::int32 newServantId = newVirtualAddress();
    _vas.insert( objToAddress( instance, newServantId ) );
    
    _servants[newServantId] = new Servant( this, instance );
    _remoteRefCounting[newServantId] = 1; // TODO set referer
    
    return newServantId;
}

void Node::releaseInstance( co::int32 instanceID )
{
    co::IObject* instance = _servants[instanceID]->getObject();
    VirtualAddresses::iterator it = _vas.find( instance );
    _vas.erase( it );
    
    delete _servants[instanceID];
    _freedIds.push( instanceID );
    
}

reef::ITransport* Node::getTransportService()
{
    return _transport;
}

void Node::setTransportService( reef::ITransport* transport )
{
    _transport = transport;
}
    
CORAL_EXPORT_COMPONENT( Node, Node );
    
} // namespace reef