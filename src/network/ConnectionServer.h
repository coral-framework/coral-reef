#ifndef _REEF_NETWORK_CONNECTION_SERVER_H_
#define _REEF_NETWORK_CONNECTION_SERVER_H_

#include <string>
#include "Connection.h"

namespace reef {
    
// Manages incomming connections
class ConnectionServer
{
public:
    ConnectionServer( const std::string& bindAddress );
    
    bool startListening();
    void stopListeneing();
    
    //! Waits until a new connection establishes
    Connection* waitForConnection();
};

} // namespace reef

#endif
