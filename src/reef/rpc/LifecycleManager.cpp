#include "LifecycleManager.h"

namespace reef {
namespace rpc {
 
co::int32 LifecycleManager::addInstance( co::IObject* instance, const std::string refererEndpoint )
{
    return 0;
}

void LifecycleManager::incrementReference( co::IObject* instance, const std::string refererEndpoint )
{
    
}

void LifecycleManager::decrementReference( co::IObject* instance, const std::string refererEndpoint )
{
    
}

void LifecycleManager::getInvoker( co::int32 instanceID )
{
    
}

co::int32 LifecycleManager::getInstanceID( co::IObject* instance )
{
    return 0;
}
        
}
}
