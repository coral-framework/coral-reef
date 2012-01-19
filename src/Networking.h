#ifndef NETWORK_H
#define NETWORK_H

#include <co/Coral.h>
#include <stdio.h>

namespace reef
{
    
struct Message
{
    char type;
    void* data;
	size_t bytes;
    co::int32 senderType; // Proxy = 0, Servant = 1 or Node = 2;
    co::int32 senderId;
};

class Network
{
public:
    static int broadcastMsg( Message& msg );
    
    static int sendMsg( int rmtNodeId, Message& msg );
    
    static bool getMessage( Message*& msg, co::int32& rmtNodeId );
    
private:
};

}

#endif