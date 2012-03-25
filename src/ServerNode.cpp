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
    _channels.push_back( servant );
    _instances.push_back( 0 );
}
    
void ServerNode::update()
{
    if( !_binder.isBound() )
        return;
        
	std::string message;
	if( _binder.receive( message ) )    
	{
		// Route the message to the proper channel
		_decoder.routeAndDeliver( message, _channels );
	}
}

void ServerNode::stop()
{
    if( _binder.isBound() )
        _binder.close();
    
    // fill the empty holes in the channels vector
    for( ; !_freedIds.empty(); _freedIds.pop() )
    {
        if( _freedIds.top() != _channels.size() )
            _channels[_freedIds.top()] = _channels.back();
        
        _channels.pop_back();
    }
    
    // now delete all the servants
    size_t size = _channels.size();
    for( int i = 0; i < size; i++ )
    {
        delete static_cast<Servant*>( _channels[i] );
    }
}

int ServerNode::newInstance( const std::string& typeName ) 
{
	co::IObject* instance = co::newInstance( typeName );

    Servant* servant = new Servant( instance );
    co::int32 newChannelId;
    
    if( !_freedIds.empty() )
    {
        newChannelId = _freedIds.top();
        _channels[newChannelId];
        _freedIds.pop();
    }
    else
    {
        _channels.push_back( servant );
        newChannelId = _channels.size() - 1;
    }
    
    _instances.push_back( instance );
    _vas.insert( objToAddress( instance, newChannelId ) );
        
    return newChannelId;
}

void ServerNode::removeInstance( co::int32 instanceId )
{
    delete _channels[instanceId];
    _freedIds.push( instanceId );
    
}

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef