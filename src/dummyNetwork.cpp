#include "Networking.h"

#include <string.h>

namespace reef
{
    
// Only for the dummy implementation, will be removed later
static std::vector<Message*> s_dummyMsgList;
static std::vector<co::int32> s_rmtNodesLocalIds;
    
int Network::broadcastMsg( Message& msg )
{
    return 0;
}

int Network::sendMsg( int rmtNodeId, Message& msg )
{
    // copy the msg
    Message* heapMsg = new Message();
    heapMsg->type = msg.type;
    heapMsg->bytes = msg.bytes;
    //memcpy( heapMsg->data, msg.data, msg.bytes );
    
    // "send it"
    s_dummyMsgList.push_back( heapMsg );
    s_rmtNodesLocalIds.push_back( rmtNodeId );
    
    return 1;
}

bool Network::getMessage( Message*& msg, co::int32& rmtNodeId )
{
    if( s_dummyMsgList.empty() )
        return false;
    
    msg = s_dummyMsgList.back();
    rmtNodeId = s_rmtNodesLocalIds.back();
    s_dummyMsgList.pop_back();
    s_rmtNodesLocalIds.pop_back();
    
    return true;
}

}