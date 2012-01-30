#include "ClientNode_Base.h"
#include "RemoteObject.h"

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
    
    // ------ reef.IClientNode Methods ------ //
    
    co::IObject* newRemoteInstance( const std::string& componentTypeName, const std::string& address )
    {
        co::RefPtr<reef::RemoteObject> obj( new reef::RemoteObject() );
        co::IComponent* componentType = co::cast<co::IComponent>( co::getType( componentTypeName ) );
        return 0;
    }
    
private:
    std::map<std::string, co::int32> _proxyToServant; 
    
};

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef
