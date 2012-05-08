#include "ZMQActiveLink.h"

#include "ZMQTransport.h"

namespace zmq {

ZMQActiveLink::ZMQActiveLink( ZMQTransport* creator ) :
    _context( 1 ), _socket( _context, ZMQ_DEALER ), _creator( creator )
{
}

ZMQActiveLink::ZMQActiveLink()  : _context( 1 ), _socket( _context, ZMQ_DEALER )
{
    // empty constructor
}

ZMQActiveLink::~ZMQActiveLink()
{
    _creator->onLinkDestructor( _address );
}

bool ZMQActiveLink::connect( const std::string& address )
{
    _address = address;
    _socket.connect( address.c_str() );
    return true;
}
    
// ------ reef.IActiveLink Methods ------ //

bool ZMQActiveLink::receiveReply( std::string& msg )
{
    zmq::message_t zmsg;
    if( _socket.recv( &zmsg, ZMQ_NOBLOCK ) )
    {
        msg.resize( zmsg.size() );
        memcpy( &msg[0], zmsg.data(), zmsg.size() );
        return true;
    }
    
    return false; 
}

void ZMQActiveLink::send( const std::string& msg )
{
    zmq::message_t zmsg( msg.size() );
    memcpy( zmsg.data(), msg.data(), msg.size() );
    _socket.send( zmsg );
}


CORAL_EXPORT_COMPONENT( ZMQActiveLink, ZMQActiveLink );

} // namespace reef
