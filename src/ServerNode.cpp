#include "ServerNode.h"

#include "Decoder.h"
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
        
	_decoder = new Decoder( _binder );
	Servant* servant = new Servant( 0 );
    servant->setServerNode( this );
    _channels.push_back( servant );
}
    
void ServerNode::update()
{
    if( !_binder->isBinded() )
        return;
        
	std::string message;
	if( _binder->receive( message ) )    
	{
		// Route the message to the proper channel
		_decoder->routeAndDeliver( message, _channels );
	}
}

void ServerNode::stop()
{
    if( _binder )
        _binder->close();
    
}
       
// DecoderChannel
int ServerNode::newInstance( const std::string& typeName ) 
{
	co::IObject* instance = co::newInstance( typeName );

    _channels.push_back( new Servant( instance ) );

    co::int32 newChannelId = _channels.size() - 1;
    
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