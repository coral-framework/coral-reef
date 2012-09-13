#include "ZMQPassiveLink.h"

#include <string.h>

namespace zmq {

ZMQPassiveLink::ZMQPassiveLink( zmq::context_t& context ) : _socket( context, ZMQ_ROUTER )
{
}

ZMQPassiveLink::ZMQPassiveLink() : _socket( *((zmq::context_t*)0), ZMQ_ROUTER )
{
}

ZMQPassiveLink::~ZMQPassiveLink()
{
    _socket.close();
}
    
bool ZMQPassiveLink::bind( const std::string& address )
{
    _address = address;
    _socket.bind( address.c_str() );
    return true;
}

// ------ reef.IPassiveLink Methods ------ //

bool ZMQPassiveLink::receive( std::string& msg )
{
    // check if there is a message and save its sender
    if( _socket.recv( &_lastSender, ZMQ_NOBLOCK ) )
    {
        // get the actual message
        zmq::message_t zmsg;
        _socket.recv( &zmsg );
        msg.resize( zmsg.size() );
        memcpy( &msg[0], zmsg.data(), zmsg.size() );
        return true;
    }
    else if( zmq_errno() == EAGAIN )
    {
        return false;
    }
    else
    {
        //TODO: exception
        assert( false );
        return false;
    }
}

void ZMQPassiveLink::sendReply( const std::string& msg )
{
    _socket.send( _lastSender, ZMQ_SNDMORE );
    
    zmq::message_t zmsg( msg.size() );
    memcpy( zmsg.data(), msg.data(), msg.size() );
    _socket.send( zmsg );
}

CORAL_EXPORT_COMPONENT( ZMQPassiveLink, ZMQPassiveLink );

} // namespace reef
