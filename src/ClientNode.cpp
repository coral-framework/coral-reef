#include "ClientNode_Base.h"
#include "RemoteObject.h"
#include "Encoder.h"
#include "network/Transport.h"

#include <co/IObject.h>

#include <map>

namespace reef {
 
class IServerNode;
    
class ClientNode : public ClientNode_Base
{
public:
    ClientNode()
    {
        _transport = Transport::getInstance();
    }
    
    virtual ~ClientNode()
    {
    }
    
    co::IObject* newRemoteInstance( const std::string& componentTypeName, 
                                   const std::string& address )
    {
        co::IComponent* componentType = co::cast<co::IComponent>( 
                                          co::getType( componentTypeName ) );
        
        Connecter* connecter = _transport->getConnecter( address );
        
        return new RemoteObject( componentType, new Encoder( connecter,
                                        _serverNode ) );
    }
    
private:
    Transport* _transport;
    IServerNode* _serverNode;
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef