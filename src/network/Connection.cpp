#include "Connection.h"

namespace reef {
    
Connection::Connection( const std::string& type )
{
    // empty
}

Connection::~Connection()
{
    // empty
}

bool Connection::establish( const std::string& address )
{
    return false;
}
    
void Connection::close()
{
    
}
    
void Connection::send( const Message& message )
{
    
}

void Connection::receive( Message& message, int timeout )
{
    
}
    
} // namespace reef
