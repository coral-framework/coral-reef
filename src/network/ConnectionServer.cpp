#include "ConnectionServer.h"

#include <stdio.h>

namespace reef
{

ConnectionServer::ConnectionServer( const std::string& bindAddress )
{
    _address = bindAddress;
}
    
} // namespace reef
