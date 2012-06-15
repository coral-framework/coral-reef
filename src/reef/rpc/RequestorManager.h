#ifndef _REEF_REQUESTORMANAGER_H_
#define _REEF_REQUESTORMANAGER_H_

#include <map>
#include <string>

namespace reef {
namespace rpc {

class Node;
class Requestor;
class ITransport;

class RequestorManager
{
public:
    
    RequestorManager( Node* node, ITransport* transport, const std::string& localEndpoint );
    
    ~RequestorManager();
    
    Requestor* getOrCreateRequestor( const std::string& endpoint );
    
    void onRequestorDestroyed( const std::string& endpoint );
    
    inline Node* getNode() { return _node; }
    
private:
    Node* _node;
    ITransport* _transport;
    std::string _localEndpoint;
    std::map<std::string, Requestor*> _requestors;
};

}
}

#endif