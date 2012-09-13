#include "ZMQTransport.h"

#include "ZMQActiveLink.h"
#include "ZMQPassiveLink.h"

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>

namespace zmq {
    
ZMQTransport::ZMQTransport() : _context( 1 )
{
    // empty constructor
}

ZMQTransport::~ZMQTransport()
{
    // empty destructor
}

// ------ reef.ITransport Methods ------ //

reef::rpc::IPassiveLink* ZMQTransport::bind( const std::string& addressToListen )
{
    ZMQPassiveLink* link = new ZMQPassiveLink( _context );
    link->bind( addressToListen );
    return link;
}

reef::rpc::IActiveLink* ZMQTransport::connect( const std::string& addressToConnect )
{
    ZMQActiveLink* link = new ZMQActiveLink( _context );
    link->connect( addressToConnect );
    return link;
}
     
CORAL_EXPORT_COMPONENT( ZMQTransport, ZMQTransport );
    
} // namespace reef
