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
        _mainConnection = new Connection( "" );
        _mainConnection->bind( address );
        
        _channels.push_back( new OutputChannel( _mainConnection, this ) );
        _channels[0]->setId( 0 );
        
        while( true )
        {
            std::string message;
            _mainConnection->receive( message );
            
            std::cerr << "Received " << message << std::endl;
            fflush( stderr );
            
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
        Channel* newChannel = new OutputChannel( _mainConnection, new Servant( typeName ) );
        newChannel->setId( _channels.size() );
        _channels.push_back( newChannel );
        
        return newChannel->getId();
    }
    
private:
    Connection* _mainConnection;
    
    typedef std::vector<Channel*> Channels;
    typedef std::vector<Connection*> RemoteConnections;
    
    Channels _channels;
    RemoteConnections _connections;
};

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef
