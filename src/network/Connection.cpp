#include "Connection.h"

#include <string.h>

#include <algorithm>

namespace reef {
    
Connection::Connection( const std::string& type ) 
    : _context( 1 )
{
    if( type == "REP" )
    {
        _socket = new zmq::socket_t( _context, ZMQ_REP );
    }
    else if( type == "REQ" )
    {
        _socket = new zmq::socket_t( _context, ZMQ_REP );
    }
}

Connection::~Connection()
{
    delete _socket;
}

bool Connection::bind( const std::string& address )
{
    _socket->bind( address.c_str() );
    return true;
}
    
bool Connection::connect( const std::string& address )
{
    _socket->connect( address.c_str() );
    return true;
}
    
void Connection::close()
{
    _socket->close();
}
    
void Connection::send( const std::string& data )
{
    zmq::message_t msg( data.size() );
    memcpy( msg.data(), data.data(), data.size() );
    _socket->send( msg );
}

void Connection::receive( std::string& data, int timeout )
{
    zmq::message_t msg;
    _socket->recv( &msg );
    data.resize( msg.size() );
    memcpy( &data[0], msg.data(), msg.size() );
}
    
} // namespace reef