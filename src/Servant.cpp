#include "Servant.h"

#include <co/IPort.h>
#include <co/IReflector.h>

#include <string>

namespace reef
{

Servant::Servant( const std::string& type )
{
    _object = co::newInstance( type );
}
    
void Servant::onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    co::Range<co::IPort* const> ports = _object->getComponent()->getPorts();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::Any ret;
    co::IService* service = _object->getServiceAt( port );
    service->getFacet()->getType()->getReflector()->invoke( service, method, args, ret );
}
    
void Servant::onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    
}
    
void Servant::onGetField( Channel* channel, co::int32 serviceId, co::IField* field, co::Any& result )
{
    
}
    
void Servant::onSetField( Channel* channel, co::int32 serviceId, co::IField* field, const co::Any& value )
{
    
}

    
} // namespace reef


