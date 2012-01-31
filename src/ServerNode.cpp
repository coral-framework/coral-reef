#include "ServerNode_Base.h"

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
    
private:
    // member variables
};

CORAL_EXPORT_COMPONENT( ServerNode, ServerNode );
    
} // namespace reef
