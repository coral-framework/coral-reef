#include "InstanceManager.h"

#include "InstanceContainer.h"

#include <rpc/RemotingException.h>

#include <co/Log.h>
#include <co/Coral.h>
#include <co/Exception.h>

#include <sstream>

namespace rpc {

InstanceManager::InstanceManager()
{
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
}
    
co::int32 InstanceManager::publishInstance( co::IObject* instance, const std::string& key )
{
    co::int32 instanceID = newID();
    _instances[instanceID] = new InstanceContainer( instance );

    _published.insert( std::pair<std::string, co::int32>( key, instanceID ) );
    
    return instanceID;
}
    
void InstanceManager::unpublishInstance( const std::string& key )
{
    std::map<std::string, co::int32>::iterator it = _published.find( key );
    
    assert( it != _published.end() );
    
    releaseInstance( it->second );
}

co::int32 InstanceManager::findInstance( const std::string& key, const std::string& instanceType,
                                        const std::string& lesseeEndpoint ) 
{
    std::map<std::string, co::int32>::iterator it = _published.find( key );
    
    if( it == _published.end() )
    {
        CORAL_LOG( WARNING ) << "Node: " << lesseeEndpoint << " requested the invalid key: " << key;
        return -1;
    }
    
    co::IComponent* component = _instances[it->second]->getComponent();
    if( component->getFullName() != instanceType )
        CORAL_THROW( RemotingException, "Published instance type " << component->getFullName() 
        << " differs from requested type " << instanceType );
    
    return it->second;
}

co::int32 InstanceManager::addInstance( co::IObject* instance, const std::string& lesseeEndpoint ) 
{    
    co::int32 instanceID = newID();
    _instances[instanceID] = new InstanceContainer( instance );
    
    return instanceID;
}
 
InstanceContainer* InstanceManager::getInstance( co::int32 instanceID )
{ 
    std::set<co::int32>::iterator it = _freedIds.find( instanceID );
    
    // Check if not a freed ID or higher then ID count. Can still be acessing the wrong instance.
    if( it == _freedIds.end() && instanceID < _instances.size() )
        return _instances[instanceID]; 
    
    return 0;
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

} // namespace rpc