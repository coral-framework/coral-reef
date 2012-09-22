#include "ZMQTransport.h"

#include "ZMQConnector.h"
#include "ZMQAcceptor.h"

#include <rpc/IConnector.h>
#include <rpc/IAcceptor.h>

namespace zmq {
    
ZMQTransport::ZMQTransport() : _context( 1 )
{
    // empty constructor
}

ZMQTransport::~ZMQTransport()
{
    // empty destructor
}

// ------ rpc.ITransport Methods ------ //

rpc::IAcceptor* ZMQTransport::bind( const std::string& addressToListen )
{
    ZMQAcceptor* link = new ZMQAcceptor( _context );
    link->bind( addressToListen );
    return link;
}

rpc::IConnector* ZMQTransport::connect( const std::string& addressToConnect )
{
    ZMQConnector* link = new ZMQConnector( _context );
    link->connect( addressToConnect );
    return link;
}
     
CORAL_EXPORT_COMPONENT( ZMQTransport, ZMQTransport );
    
} // namespace zmq
