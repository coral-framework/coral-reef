#include "ClientNode_Base.h"
#include "RemoteObject.h"
#include "Channel.h"
#include "network/Connection.h"

#include <co/IObject.h>

#include <map>

namespace reef {
    
class ClientNode : public ClientNode_Base
{
public:
    ClientNode()
    {
        // empty constructor
    }
    
    virtual ~ClientNode()
    {
        // empty destructor
    }
    
    co::IObject* newRemoteInstance( const std::string& componentTypeName, const std::string& address )
    {
        Connection* connection = getOrCreateConnection( address );
        Channel* objChannel = new Channel( connection );
        objChannel->establish( componentTypeName );
        
        co::IComponent* componentType = co::cast<co::IComponent>( co::getType( componentTypeName ) );
        return new reef::RemoteObject( componentType, objChannel );
    }
    
    // Retrieves a new connection (if there's no connection stablished for the given address)
    // or the corresponding connection pointer otherwise.
    Connection* getOrCreateConnection( const std::string& address )
    {
        int connectionSlot = -1;
        for( int i = 0; i < _connections.size(); ++i )
        {
            if( _connections[i]->getAddress() == address )
            {
                connectionSlot = i;
                break;
            }
        }
        
        Connection* connection = 0;
        if( connectionSlot == -1 )
        {
            connection = new Connection( "CON_TYPE" );
            if( !connection->establish( address ) )
            {
                // TODO: throw exception?
            }
            
            _connections.push_back( connection );
        }
        else
            connection = _connections[connectionSlot];

              
        return connection;
    }
                                          
private:
    typedef std::vector<Channel*> Channels;
    typedef std::vector<Connection*> RemoteConnections;

    Channels _channels;
    RemoteConnections _connections;
    
    typedef std::map<int,int> SwitchBoard;
    SwitchBoard _switchBoard;
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef
