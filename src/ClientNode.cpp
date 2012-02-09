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
        Connecter* connection = new Connecter();
        connection->connect( address );
    
        co::IComponent* componentType = co::cast<co::IComponent>( co::getType( componentTypeName ) );
        return new reef::RemoteObject( componentType, new InputChannel( connection ) );
    }                     
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef