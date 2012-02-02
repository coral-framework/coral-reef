#include "Connection.h"

#include <string.h>

namespace reef {
    
    Connection::Connection( const std::string& type ) 
    : _context( 1 ), _socket( _context, ZMQ_PULL )
{
    // empty
}

Connection::~Connection()
{
    // empty
}

bool Connection::bind( const std::string& address )
{
    _socket.bind ( address.c_str() );
    return true;
}
    
void Connection::close()
{
    _socket.close();
}
    
void Connection::send( const Message& message )
{
    zmq::message_t msg ( message.size() );
    memcpy( msg.data(), message.c_str(), message.size() * sizeof( char ) );
    _socket.send( msg );
}

void Connection::receive( Message& message, int timeout )
{
    zmq::message_t msg;
    _socket.recv( &msg );
    
    message = reinterpret_cast<char*>( msg.data() );
}
    
} // namespace reef