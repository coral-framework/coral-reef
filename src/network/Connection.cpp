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
    
    void Connection::send( const std::string& msgData )
{
    zmq::message_t msg ( msgData.size() );
    memcpy( msg.data(), msgData.c_str(), msgData.size() * sizeof( char ) );
    _socket.send( msg );
}

    void Connection::receive( std::string& msgData, int timeout )
{
    zmq::message_t msg;
    _socket.recv( &msg );
    
    msgData = reinterpret_cast<char*>( msg.data() );
}
    
} // namespace reef