#include "Node.h"

#include "Decoder.h"
#include "Servant.h"
#include "RemoteObject.h"
#include "network/Connection.h"

#include <co/Exception.h>

#include <iostream>
#include <map>

namespace reef {

Node* Node::_nodeInstance = 0;
    
Node::Node()
{
    if( _nodeInstance )
        throw new co::Exception( "Only one server node allowed at a time" );
    
    _nodeInstance = this;
}
    
Node::~Node()
{
    _nodeInstance = 0;
}
    
co::IObject* Node::newRemoteInstance( const std::string& instanceType, 
                                           const std::string& address )
{
    Connecter* connecter = Connecter::getOrOpenConnection( address );
    co::int32 instanceID = requestNewInstance( connecter, instanceType );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return RemoteObject::getOrCreateRemoteObject( component, connecter, instanceID );
}

void Node::start( const std::string&  boundAddress, const std::string& connectableAddress )
{
    _binder.bind( boundAddress );
    
    _myPublicAddress = connectableAddress;
    
    // This first instanceID is for the Node
    _servants.push_back( 0 );
    _remoteRefCounting.push_back( 0 );
}
    
void Node::update()
{
    if( !_binder.isBound() )
        throw new co::Exception( "The server node must be started" );
        
	std::string msg;
    if( _binder.receive( msg ) )
        dispatchMessage( msg );
}

void Node::stop()
{
    if( _binder.isBound() )
        _binder.close();
    
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
        delete static_cast<Servant*>( _servants[i] );
    }
}

co::IObject* Node::getRemoteInstance( const std::string& instanceType, co::int32 instanceID, 
                                     const std::string& ownerAddress )
{
    Connecter* connecter = Connecter::getOrOpenConnection( ownerAddress );
    
    co::IComponent* component = co::cast<co::IComponent>( co::getType( instanceType ) );
    
    return RemoteObject::getOrCreateRemoteObject( component, connecter, instanceID );
}

co::int32 Node::requestNewInstance( Connecter* connecter, const std::string& componentName )
{
    std::string msg;
    _encoder.encodeNewInstMsg( componentName, msg );
    connecter->send( msg );
    
    while( !connecter->receiveReply( msg ) )
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
    _binder.reply( msg );
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
    
    _binder.reply( msg );
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
    
    _servants[newServantId] = new Servant( instance );
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

CORAL_EXPORT_COMPONENT( Node, Node );
    
} // namespace reef