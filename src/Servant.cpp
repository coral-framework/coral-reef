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
}
    
void Servant::onSendCall( Channel* channel, co::int32 serviceId, co::int32 memberIndex, co::Range<co::Any const> args )
{
    co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::Any ret;
    co::IService* service = _object->getService( "toto" );

    co::IInterface* itf = service->getInterface();
    co::IMember* member = itf->getMembers()[memberIndex];
    assert( member->getKind() == co::MemberKind::MK_METHOD );
    
    co::IMethod* method = co::cast<co::IMethod>( member );

    co::IReflector* ref = itf->getReflector();
    ref->invoke( service, method, args, ret );
}
    
void Servant::onCall( Channel* channel, co::int32 serviceId, co::int32 memberIndex, co::Range<co::Any const> args, co::Any& result )
{
    
}
    
void Servant::onGetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    
}
    
void Servant::onSetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    
}

    
} // namespace reef


