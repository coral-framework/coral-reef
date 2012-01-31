#include "Connection.h"

namespace reef {
    
Connection::Connection( const std::string& type, const std::string& address )
{
    _address = address; 
}

Connection::~Connection()
{
    // empty
}

void Connection::send( const Message& message )
{
    
}

void Connection::receive( Message& message )
{
    
}
    
} // namespace reef
