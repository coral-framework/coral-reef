#include "ClientNode_Base.h"
#include "RemoteObject.h"
#include "Encoder.h"
#include "network/Connection.h"

#include <co/IObject.h>

#include <map>

namespace reef {
 
class IServerNode;
    
class ClientNode : public ClientNode_Base
{
public:
    ClientNode()
    {
        // empty constructor
    }
    
    virtual ~ClientNode()
    {
    }
    
    co::IObject* newRemoteInstance( const std::string& componentTypeName, 
                                   const std::string& address )
    {
        co::IComponent* componentType = co::cast<co::IComponent>( 
                                          co::getType( componentTypeName ) );
        
        Connecter* connecter = Connecter::getOrOpenConnection( address );
        
        return new RemoteObject( componentType, new Encoder( connecter,
                                        _serverNode ) );
    }
    
private:
    IServerNode* _serverNode;
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef