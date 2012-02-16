#include "Servant.h"

#include <co/IPort.h>
#include <co/IReflector.h>
#include <co/IMethod.h>
#include <co/IMember.h>

#include <string>
#include <iostream>

#include "ServerNode.h"

namespace reef
{

Servant::Servant( co::IObject* object )
{
    _object = object;
	co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
	_openedServices.resize( ports.getSize() );
	_openedReflectors.resize( ports.getSize() );
}
    
void Servant::onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
	if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
		
	_openedReflectors[serviceId]->invoke( _openedServices[serviceId], method, args, co::Any() );
}
    
void Servant::onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
		
	_openedReflectors[serviceId]->invoke( _openedServices[serviceId], method, args, result );		
}
    
void Servant::onGetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    
}
    
void Servant::onSetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    
}

void Servant::onServiceFirstAccess( co::int32 serviceId )
{
	co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::IService* service = _object->getServiceAt( port );

    co::IInterface* itf = service->getInterface();

    co::IReflector* reflector = itf->getReflector();

	// saving for easier later access
	_openedServices[serviceId] = service;
    _openedReflectors[serviceId] = reflector;
}
    
} // namespace reef


