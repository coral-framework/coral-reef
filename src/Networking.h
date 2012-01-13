#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>

namespace reef
{
    
struct Message
{
	void* buffer;
	size_t bytes;
};

class Network
{
public:
    static int broadcastMsg( Message& msg );
    
    static int sendMsg( int address, Message& msg );
    
    static Message* getMessage();
    
};

}

#endif