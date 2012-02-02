#include "Servant.h"

#include <co/IPort.h>
#include <co/IReflector.h>
#include <co/IMethod.h>

#include <string>
#include <iostream>

namespace reef
{

Servant::Servant( const std::string& type )
{
    _object = co::newInstance( type );
}
    
void Servant::onSendCall( Channel* channel, co::int32 facetId, co::int32 methodId, co::Range<co::Any const> args )
{
    co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[facetId];
    assert( port->getIsFacet() );
    
    co::Any ret;
    co::IService* service = _object->getService( "toto" );

    co::IInterface* typ = service->getInterface();
    std::cerr << "Interface: " << typ->getName().c_str() << std::endl;
    co::IReflector* ref = typ->getReflector();
    co::IMethod* met = typ->getMethods()[1];
    std::cerr << "Method: " << met->getName().c_str() << std::endl;
    
    ref->invoke( service, met, args, ret );
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


