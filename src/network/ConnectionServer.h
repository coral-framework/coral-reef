#ifndef _REEF_NETWORK_CONNECTION_SERVER_H_
#define _REEF_NETWORK_CONNECTION_SERVER_H_

#include <string>
#include "Connection.h"

#include <zmq.hpp>
#include <string>

namespace reef {
    
// Manages incomming connections
class ConnectionServer
{
public:
    ConnectionServer( const std::string& bindAddress );
    
    //! Waits until a new connection establishes
    Connecter* waitForConnection();
    
private:
    std::string _address;
};

} // namespace reef

#endif
