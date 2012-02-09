#include "ServerNode_Base.h"

#include "Channel.h"
#include "Servant.h"

#include "network/Connection.h"
#include "network/ConnectionServer.h"

#include <iostream>
#include <map>

namespace reef {
    
class ServerNode : public ServerNode_Base, public OutputChannelDelegate
{
public:
    ServerNode()
    {
        // empty constructor
    }
    
    virtual ~ServerNode()
    {
        // empty destructor
    }
    
    void start( const std::string& address )
    {
        _binder = new Binder();
        _binder->bind( address );
        
        _channels.push_back( new OutputChannel( _binder, this ) );
        _channels[0]->setId( 0 );
    }
    
	void update()
	{
		std::string message;
        
		if( _binder->receive( message ) )    
		{
			// Route the message to the proper channel
			Channel::route( message, _channels );
		}
	}

    void stop()
    {
        // TODO: implement this method.
    }
       
    // OutputChannelDelegate
    int onNewInstance( Channel* channel, const std::string& typeName ) 
    {
        Channel* newChannel = new OutputChannel( _binder, new Servant( typeName ) );
        newChannel->setId( _channels.size() );
        _channels.push_back( newChannel );
        
        return newChannel->getId();
    }
    
private:
	Binder* _binder;
    
    typedef std::vector<Channel*> Channels;
    
    Channels _channels;
};

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef