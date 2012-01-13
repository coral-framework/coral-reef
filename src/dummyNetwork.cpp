#include "Networking.h"

namespace reef
{
    
int Network::broadcastMsg( Message& msg )
{
    return 0;
}

int Network::sendMsg( int address, Message& msg )
{
    return 0;
}

Message* Network::getMessage()
{
    static Message msg;
    return &msg;
}

}