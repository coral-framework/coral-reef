#ifndef _RPC_INSTANCEMANAGER_H_
#define _RPC_INSTANCEMANAGER_H_

#include <co/IObject.h>

#include <map>
#include <set>

namespace rpc {

class InstanceContainer;

class InstanceManager
{
public:
    InstanceManager();
    ~InstanceManager();
    
    co::int32 publishInstance( co::IObject* instance, const std::string& key );
    
    void unpublishInstance( const std::string& key );
    
    co::int32 findInstance( const std::string& key, const std::string& instanceType, 
                           const std::string& lesseeEndpoint ); 
    
    co::int32 addInstance( co::IObject* instance, const std::string& lesseeEndpoint );
    
    InstanceContainer* getInstance( co::int32 instanceID );
    
private:
    co::int32 newID();
    
    void releaseInstance( co::int32 instanceID );
    
private:
    std::vector<InstanceContainer*> _instances;
    std::map<std::string, co::int32> _published;
    std::set<co::int32> _freedIds; //!< auxiliary array for recicling deleted Ids
};

} // namespace rpc

#endif