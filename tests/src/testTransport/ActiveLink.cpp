#include "ActiveLink.h"

#include "Transport.h"

namespace testTransport {

ActiveLink::ActiveLink( Transport* creator, const std::string& address ) : _address( address ), 
    _creator( creator )
{
}

ActiveLink::ActiveLink()
{
    // empty constructor
}

ActiveLink::~ActiveLink()
{
    _creator->onLinkDestructor( _address );
}
    
// ------ reef.IActiveLink Methods ------ //

bool ActiveLink::receiveReply( std::string& msg )
{
    return _creator->checkReply( _address, msg );
}

void ActiveLink::send( const std::string& msg )
{
    _creator->post( _address, msg );
}


CORAL_EXPORT_COMPONENT( ActiveLink, ActiveLink );

} // namespace reef
