#include "ServerNode.h"

#include "Decoder.h"
#include "Servant.h"

#include "network/Connection.h"

#include <iostream>
#include <map>

namespace reef {

ServerNode::ServerNode()  : _decoder( &_binder )
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
	Servant* servant = new Servant( 0 );
    servant->setServerNode( this );
    _servants.push_back( servant );
}
    
void ServerNode::update()
{
    if( !_binder.isBound() )
        return;
        
	std::string message;
	if( _binder.receive( message ) )    
	{
		// Route the message to the proper servant
		_decoder.routeAndDeliver( message, _servants );
	}
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
    for( int i = 0; i < size; i++ )
    {
        delete static_cast<Servant*>( _servants[i] );
    }
}

co::int32 ServerNode::newInstance( const std::string& typeName ) 
{
	co::IObject* instance = co::newInstance( typeName );

    return publishInstance( instance );        
}
    
co::int32 ServerNode::openRemoteReference( co::IObject* instance )
{
    co::int32 va = getVirtualAddress( instance );
    if( va != -1 )
        _remoteRefCounting[va]++;
    else
        va = publishInstance( instance );
    
    return va;
}

void ServerNode::closeRemoteReference( co::int32 virtualAddress )
{
    if( --_remoteRefCounting[virtualAddress] < 1 )
        releaseInstance( virtualAddress );
    
}

co::int32 ServerNode::getVirtualAddress( const co::IObject* instance )
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
   
co::int32 ServerNode::publishInstance( co::IObject* instance )
{
    co::int32 newServantId = newVirtualAddress();
    _vas.insert( objToAddress( instance, newServantId ) );
    
    _servants[newServantId] = new Servant( instance );
    _remoteRefCounting[newServantId] = 1;
    
    return newServantId;
}
    

void ServerNode::releaseInstance( co::int32 virtualAddress )
{
    co::IObject* instance = _servants[virtualAddress]->getObject();
    VirtualAddresses::iterator it = _vas.find( instance );
    _vas.erase( it );
    
    delete _servants[virtualAddress];
    _freedIds.push( virtualAddress );
    
}

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef