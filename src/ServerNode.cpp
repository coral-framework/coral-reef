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
    ServerNode() : _server( 0 )
    {
        // empty constructor
        _channels.push_back( new OutputChannel( this ) );
    }
    
    virtual ~ServerNode()
    {
        // empty destructor
    }
    
    void start( const std::string& address )
    {
         if( !_server )
             _server = new ConnectionServer( address );
        
        Connection* c = new Connection( "" );
        c->bind( address );
        while( true )
        {
            std::string msg;
            c->receive( msg );
            
            std::cerr << "Received " << msg << std::endl;
            fflush( stderr );
            
            Channel::MessageInfo mi = Channel::getDestination( msg );
            std::cerr << "Desgination: " << mi.destination << std::endl;
            fflush( stderr );
            _channels[mi.destination]->write( mi.message );
        }
    }
    
    void stop()
    {
        // TODO: implement this method.
    }
       
    // OutputChannelDelegate
    void onNewInstance( Channel* channel, const std::string& typeName ) 
    {
        Servant* servant = new Servant( typeName );
        Channel* newChannel = new OutputChannel( servant );
        _channels.push_back( newChannel );
    }
    
private:
    ConnectionServer* _server;
    
    typedef std::vector<Connection*> RemoteConnections;
    
    RemoteConnections _connections;
};

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef
