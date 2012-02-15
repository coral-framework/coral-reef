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
}
    
void Servant::onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{/*
 TODO: make a test with this code
	for( ; args; args.popFirst() )
	{
		co::Any any = args.getFirst();
		co::TypeKind tp = any.getKind();
		co::Range<const co::int32> num = any.get<co::Range<const co::int32> >();
		for( ; num; num.popFirst() )
		{
			co::int32 a = num.getFirst();
		}
	}*/
	if( _openedServices[serviceId] )
	{
		ref->invoke( _openedServices[serviceId], method, args, co::Any() );
	}
    co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::Any ret;
    co::IService* service = _object->getServiceAt( port );

    co::IInterface* itf = service->getInterface();

    co::IReflector* ref = itf->getReflector();
    ref->invoke( service, method, args, ret );
}
    
void Servant::onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    
}
    
void Servant::onGetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    
}
    
void Servant::onSetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    
}

    
} // namespace reef


