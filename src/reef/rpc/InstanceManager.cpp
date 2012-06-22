#include "InstanceManager.h"

#include "LeaseManager.h"
#include "InstanceContainer.h"

#include <co/Coral.h>

namespace reef {
namespace rpc {

InstanceManager::InstanceManager()
{
    _leaseMan = new LeaseManager();
    _instances.push_back( 0 );
}

InstanceManager::~InstanceManager()
{
    // fill the empty holes in the invokers vector
    for( ; !_freedIds.empty(); _freedIds.pop() )
    {
        if( _freedIds.top() <= _instances.size() )
            _instances[_freedIds.top()] = _instances.back();
        
        _instances.pop_back();
    }
    
    // now delete all the invokers
    size_t size = _instances.size();
    for( int i = 1; i < size; i++ )
    {
        delete _instances[i];
    }

    delete _leaseMan;
}
    
void InstanceManager::publishInstance( co::IObject* instance, const std::string& key )
{
    co::int32 instanceID = newID();
    _instances[instanceID] = new InstanceContainer( instance );
    
    createLease( instanceID, "self" );
    
    _published.insert( std::pair<std::string, co::int32>( key, instanceID ) );
}
    
void InstanceManager::unpublishInstance( const std::string& key )
{
    std::map<std::string, co::int32>::iterator it = _published.find( key );
    
    assert( it != _published.end() );
    
    cancelLease( it->second, "self" );
}

co::int32 InstanceManager::findInstance( const std::string& key, const std::string& lesseeEndpoint ) 
{
    std::map<std::string, co::int32>::iterator it = _published.find( key );
    
    assert( it != _published.end() ); //REMOTINGERROR not found
    
    createLease( it->second, lesseeEndpoint );
    
    return it->second;
}

co::int32 InstanceManager::addInstance( co::IObject* instance, const std::string& lesseeEndpoint ) 
{    
    co::int32 instanceID = newID();
    _instances[instanceID] = new InstanceContainer( instance );
    
    _leaseMan->addLease( instanceID, lesseeEndpoint );
    
    return instanceID;
}

void InstanceManager::createLease( co::int32 instanceID, const std::string& lesseeEndpoint )
{
    _leaseMan->addLease( instanceID, lesseeEndpoint );
}

void InstanceManager::cancelLease( co::int32 instanceID, const std::string& lesseeEndpoint )
{
    if( _leaseMan->removeLease( instanceID, lesseeEndpoint ) )
    {
        releaseInstance( instanceID );
    }
}
        
co::int32 InstanceManager::newID()
{
    if( !_freedIds.empty() )
    {
        co::int32 newID = _freedIds.top();
        _freedIds.pop();
        return newID;
    }
    else
    {
        _instances.push_back( 0 );
        return _instances.size() - 1;
    }
    
}
    
void InstanceManager::releaseInstance( co::int32 instanceID )
{
    delete _instances[instanceID];
    _freedIds.push( instanceID );
}

}
}