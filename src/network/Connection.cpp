#include "Connection.h"

#include <string.h>

#include <algorithm>

namespace reef {
    
Connection::Connection( const std::string& type ) 
    : _context( 1 ), _socket( _context, ZMQ_REP )
{
    // empty
}

Connection::~Connection()
{
    // empty
}

bool Connection::bind( const std::string& address )
{
    _socket.bind( address.c_str() );
    return true;
}
    
bool Connection::connect( const std::string& address )
{
    _socket.connect( address.c_str() );
    return true;
}
    
void Connection::close()
{
    _socket.close();
}
    
void Connection::send( const void* data, unsigned int size )
{
    zmq::message_t msg ( size );
    memcpy( msg.data(), data, size );
    _socket.send( msg );
}

void Connection::receive( void* data, unsigned int size, int timeout )
{
    zmq::message_t msg;
    _socket.recv( &msg );
    memcpy( data, msg.data(), std::min<int>( size, msg.size() ) );
}
    
} // namespace reef