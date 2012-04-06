#include "ServerNode.h"

#include "Decoder.h"
#include "Servant.h"
#include "network/Connection.h"

#include <co/Exception.h>

#include <iostream>
#include <map>

namespace reef {

ServerNode::ServerNode()
{
    // empty constructor
}
    
ServerNode::~ServerNode()
{
    // empty destructor
}
    
void ServerNode::start( const std::string& address )
{
    _binder.bind( address );
    
    // This first instanceID is for the ServerNode
    _servants.push_back( 0 );
}
    
void ServerNode::update()
{
    if( !_binder.isBound() )
        throw new co::Exception( "The server node must be started" );
        
	std::string msg;
    _binder.receive( msg );
    
    dispatchMessage( msg );
}

void ServerNode::stop()
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

void ServerNode::dispatchMessage( const std::string& msg )
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
        default: // the message isn't destined to the ServerNode. Pass to the appropriate servant. 
            onMsgForServant( destInstanceID, hasReturn );
    }  
}
    
void ServerNode::onNewInstMsg() 
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
    
void ServerNode::onAccessInstMsg()
{
    std::string refererIP;
    co::int32 instanceID;
    bool increment;            
    _decoder.decodeAccessInstMsg( refererIP, instanceID, increment );
    openRemoteReference( instanceID );    
}
  
void ServerNode::onMsgForServant( co::int32 instanceID, bool hasReturn )
{
    Servant* servant = getServantFor( instanceID );
    if( !hasReturn )
    {
        servant->onCallOrField( _decoder );
        return;
    }
    
    co::Any retValue;
    servant->onCallOrField( _decoder, &retValue );
    
    std::string msg;
    _encoder.encodeData( retValue, msg );
    
    _binder.reply( msg );
}

co::int32 ServerNode::publishInstance( co::IObject* instance )
{
    co::int32 instanceID = getInstanceID( instance );
    if( instanceID != -1 )
        openRemoteReference( instanceID ); // TODO set referer
    else
        instanceID = startRemoteRefCount( instance );
    
    return instanceID;
}

void ServerNode::openRemoteReference( co::int32 instanceID )
{
    _remoteRefCounting[instanceID]++;
}
    
void ServerNode::closeRemoteReference( co::int32 instanceID )
{
    if( --_remoteRefCounting[instanceID] < 1 )
        releaseInstance( instanceID );
    
}
    
co::int32 ServerNode::getInstanceID( const co::IObject* instance )
{
    VirtualAddresses::iterator it = _vas.find( instance );
    if( it != _vas.end() )
        return (*it).second;
    
    return -1;
}
    
co::int32 ServerNode::newVirtualAddress()
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
   
co::int32 ServerNode::startRemoteRefCount( co::IObject* instance )
{
    co::int32 newServantId = newVirtualAddress();
    _vas.insert( objToAddress( instance, newServantId ) );
    
    _servants[newServantId] = new Servant( instance );
    _remoteRefCounting[newServantId] = 1; // TODO set referer
    
    return newServantId;
}

void ServerNode::releaseInstance( co::int32 instanceID )
{
    co::IObject* instance = _servants[instanceID]->getObject();
    VirtualAddresses::iterator it = _vas.find( instance );
    _vas.erase( it );
    
    delete _servants[instanceID];
    _freedIds.push( instanceID );
    
}

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef