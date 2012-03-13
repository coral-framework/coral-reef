#include "ClientNode_Base.h"
#include "RemoteObject.h"
#include "Encoder.h"
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
        Connecter* connecter;
        
        std::map<std::string, Connecter*>::iterator it = _connections.find( address );
        if( it != _connections.end() )
        {
            connecter = (*it).second;
        }
        else
        {
            connecter = new Connecter();
            connecter->connect( address );
        }            
        
        co::IComponent* componentType = co::cast<co::IComponent>( co::getType( componentTypeName ) );
        return new reef::RemoteObject( componentType, new Encoder( connecter ) );
    }   
    
private:
    std::map<std::string, Connecter*> _connections;
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef