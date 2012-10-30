#include "InstanceContainer.h"

#include <co/IPort.h>

namespace rpc {

InstanceContainer::InstanceContainer( co::IObject* instance )
{
    assert( instance );

    _instance = instance;
    _component = instance->getComponent();
    co::TSlice<co::IPort*> ports = _component->getFacets();
    co::int32 numPorts = static_cast<co::int32>( ports.getSize() );
    _openedServices.resize( numPorts );
}

co::IService* InstanceContainer::getService( co::int32 facetIdx )
{
    if( facetIdx >= _openedServices.size() )
        return 0;
    
    if( !_openedServices[facetIdx] ) // if already used before access directly
        return onServiceFirstAccess( facetIdx );
        
    return _openedServices[facetIdx];
}

co::IService* InstanceContainer::onServiceFirstAccess( co::int32 serviceId )
{
    co::TSlice<co::IPort*> ports = _component->getFacets();
    
    co::IPort* port = ports[serviceId];
    if( !port->getIsFacet() )
        return 0;
    
    co::IService* service = _instance->getServiceAt( port );
    
	_openedServices[serviceId] = service;
    
    return service;
}

} // namespace rpc