#include "ServerNode.h"

#include "Channel.h"
#include "Servant.h"

#include "network/Connection.h"

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
    _binder = new Binder();
    _binder->bind( address );
        
	OutputChannel* serverChannel = new OutputChannel( this, _binder );
	serverChannel->setDelegate( this );
    _channels.push_back( serverChannel );

    _channels[0]->setId( 0 );
}
    
void ServerNode::update()
{
	std::string message;
        
	if( _binder->receive( message ) )    
	{
		// Route the message to the proper channel
		Channel::route( message, _channels );
	}
}

void ServerNode::stop()
{
    // TODO: implement this method.
}
       
// OutputChannelDelegate
int ServerNode::onNewInstance( Channel* channel, const std::string& typeName ) 
{
	co::IObject* instance = co::newInstance( typeName );
    OutputChannel* newChannel = new OutputChannel( this, _binder );
    newChannel->setDelegate( new Servant( instance ) );

	int newChannelId = _channels.size();
	newChannel->setId( newChannelId );
    _channels.push_back( newChannel );

	registerInstance( newChannelId, instance );
        
    return newChannelId;
}

void ServerNode::registerInstance( co::int32 virtualAddress, co::IObject* object )
{
	InstanceMap::iterator it = _instanceMap.find( virtualAddress );
	assert( it == _instanceMap.end() );

	_instanceMap.insert( std::pair<int, co::IObject*>( virtualAddress, object ) );
}
    
co::IObject* ServerNode::mapInstance( co::int32 virtualAddress )
{
	InstanceMap::iterator it = _instanceMap.find( virtualAddress );
	if( it == _instanceMap.end() )
		return 0;

	return it->second;
}

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef