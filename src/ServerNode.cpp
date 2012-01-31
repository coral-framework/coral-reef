#include "ServerNode_Base.h"

#include "Channel.h"

#include "network/Connection.h"
#include "network/ConnectionServer.h"

#include <map>

namespace reef {
    
class ServerNode : public ServerNode_Base
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
        // TODO: implement this method.
    }
    
    void stop()
    {
        // TODO: implement this method.
    }
    
    void loop()
    {
        
    }
    
    void establishNewChannel( const std::string& localTypeName, Connection* connection )
    {
//        Channel* c = new InpuChannel( connection );    
//        
//        int channelId = _channels.size();
//        _channels.push_back( c );
//        c->setId( channelId );
        
    }
    
    // a new channel has been established into the given connection
    void onChannelEstablished( Channel* channel, Connection* connection )
    {
        
    }
    
    void onNewConnection( Connection* connection )
    {
        _connections.push_back( connection );
    }
    
    void onConnectionClosed( Connection* connection )
    {
        
    }
    
private:
    typedef std::vector<Channel*> Channels;
    typedef std::vector<Connection*> RemoteConnections;
    
    Channels _channels;
    RemoteConnections _connections;
    
    // map from connection -> channels
    typedef std::map<int,std::vector<int>> SwitchBoard;
    SwitchBoard _switchBoard;
};

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef
