#ifndef _REEF_REQUESTORCACHE_H_
#define _REEF_REQUESTORCACHE_H_

#include <map>
#include <string>

namespace reef {
namespace rpc {

class Node;
class Requestor;
class ITransport;

class RequestorCache
{
public:
    
    RequestorCache( Node* node, ITransport* transport, const std::string& localEndpoint );
    
    Requestor* getOrCreate( const std::string& endpoint );
    
    void onRequestorDestroyed( const std::string& endpoint );
    
private:
    Node* _node;
    ITransport* _transport;
    std::string _localEndpoint;
    std::map<std::string, Requestor*> _requestors;
};

}
}

#endif