#ifndef _REEF_LIFECYCLEMANAGER_H_
#define _REEF_LIFECYCLEMANAGER_H_

#include <co/IObject.h>

namespace reef {
namespace rpc {

class Invoker;

class LifecycleManager
{
public:
    co::int32 addInstance( co::IObject* instance, const std::string refererEndpoint );
    
    void incrementReference( co::IObject* instance, const std::string refererEndpoint );
    
    void decrementReference( co::IObject* instance, const std::string refererEndpoint );
    
    void getInvoker( co::int32 instanceID );
    
    co::int32 getInstanceID( co::IObject* instance );
};

}
}
#endif