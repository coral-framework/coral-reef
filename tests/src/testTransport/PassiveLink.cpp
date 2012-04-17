#include "PassiveLink.h"

#include "Transport.h"

namespace testTransport {

PassiveLink::PassiveLink( Transport* creator, const std::string& address ) : _address( address ), 
    _creator( creator )
{
}

PassiveLink::PassiveLink()
{
}
    
PassiveLink::~PassiveLink()
{
    _creator->onLinkDestructor( _address );
}

// ------ reef.IPassiveLink Methods ------ //

bool PassiveLink::receive( std::string& msg )
{
    return _creator->check( _address, msg );
}

void PassiveLink::sendReply( const std::string& msg )
{
    _creator->postReply( _address, msg );
}

CORAL_EXPORT_COMPONENT( PassiveLink, PassiveLink );

} // namespace reef
