#include "ZMQActiveLink.h"

#include "ZMQTransport.h"

namespace zmq {

ZMQActiveLink::ZMQActiveLink( zmq::context_t& context ) : _socket( context, ZMQ_DEALER )
{
}

ZMQActiveLink::ZMQActiveLink() : _socket( *((zmq::context_t*)0), ZMQ_DEALER )
{
    // NEVER USE THIS CONSTRUCTOR!!
}

ZMQActiveLink::~ZMQActiveLink()
{
    _socket.close();
}

bool ZMQActiveLink::connect( const std::string& address )
{
    _address = address;
    _socket.connect( address.c_str() );
    return true;
}
    
// ------ rpc.IActiveLink Methods ------ //

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

} // namespace zmq
