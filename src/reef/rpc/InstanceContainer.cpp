#include "InstanceContainer.h"

#include <co/IPort.h>
#include <co/Range.h>

namespace reef {
namespace rpc {


InstanceContainer::InstanceContainer( co::IObject* instance )
{
    assert( instance );

    _instance = instance;
    _component = instance->getComponent();
    co::Range<co::IPort* const> ports = _component->getFacets();
    co::int32 numPorts = static_cast<co::int32>( ports.getSize() );
    _openedServices.resize( numPorts );
}

co::IService* InstanceContainer::getCachedService( co::int32 facetIdx )
{
    if( !_openedServices[facetIdx] ) // if already used before access directly
        onServiceFirstAccess( facetIdx );
        
    return _openedServices[facetIdx];
}



void InstanceContainer::onServiceFirstAccess( co::int32 serviceId )
{
    co::Range<co::IPort* const> ports = _component->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::IService* service = _instance->getServiceAt( port );
    
	_openedServices[serviceId] = service;
}

}
}