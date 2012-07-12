#include "InstanceManager.h"

#include "LeaseManager.h"
#include "InstanceContainer.h"

#include <co/Coral.h>

namespace reef {
namespace rpc {

InstanceManager::InstanceManager()
{
    _leaseMan = new LeaseManager();
}

InstanceManager::~InstanceManager()
{        
    // now delete all the containers
    size_t size = _instances.size();
    for( int i = 0; i < size; i++ )
    {
		std::set<co::int32>::iterator it = _freedIds.find( i );
		if( it == _freedIds.end() )
			delete _instances[i];
    }

    delete _leaseMan;
}
    
co::int32 InstanceManager::publishInstance( co::IObject* instance, const std::string& key )
{
    co::int32 instanceID = newID();
    _instances[instanceID] = new InstanceContainer( instance );
    
    createLease( instanceID, "self" );
    
    _published.insert( std::pair<std::string, co::int32>( key, instanceID ) );
    
    return instanceID;
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
    
    if( it == _published.end() ) //REMOTINGERROR LOG not found
        return -1;
    
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
 
InstanceContainer* InstanceManager::getInstance( co::int32 instanceID )
{ 
    std::set<co::int32>::iterator it = _freedIds.find( instanceID );
    
    // Check if not a freed ID or higher then ID count. Can still be acessing the wrong instance.
    if( it == _freedIds.end() && instanceID < _instances.size() )
        return _instances[instanceID]; 
    
    return 0;
}
    
co::int32 InstanceManager::getInstanceNumLeases( co::int32 instanceID )
{ 
    return _leaseMan->numLeases( instanceID ); 
}
    
co::int32 InstanceManager::newID()
{
    if( !_freedIds.empty() )
    {
        std::set<co::int32>::iterator it = _freedIds.begin();
        co::int32 newID = *it;
        _freedIds.erase( it );
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
    _freedIds.insert( instanceID );
}

}
}