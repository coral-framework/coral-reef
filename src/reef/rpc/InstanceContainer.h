#ifndef _REEF_INSTANCECONTAINER_H_
#define _REEF_INSTANCECONTAINER_H_

#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/IService.h>
#include <co/IComponent.h>

namespace reef {
namespace rpc {

// Holds the reference for the instance and cache reflectors and facets for faster access
class InstanceContainer
{
public:
    InstanceContainer( co::IObject* instance );
    
    // Get the (cacheable) service
    co::IService* getService( co::int32 facetIdx );
    
    inline co::IObject* getInstance(){ return _instance.get(); }
    
    inline co::IComponent* getComponent(){ return _component; }
    
private:
    co::RefPtr<co::IObject> _instance;
    
    co::int32 _refCount;
    
    co::IComponent* _component;
    
    // initializes _openedService's and Reflector's index for the accessed service
	co::IService* onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
};

}
}

#endif