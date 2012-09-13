#ifndef _REEF_REQUESTORMANAGER_H_
#define _REEF_REQUESTORMANAGER_H_

#include <map>
#include <string>

namespace reef {
namespace rpc {

class Node;
class Requestor;
class ITransport;
class InstanceManager;
class ServerRequestHandler;

class RequestorManager
{
public:
    
    RequestorManager( InstanceManager* instanceMan, ITransport* transp, ServerRequestHandler* srh );
    
    ~RequestorManager();
    
    Requestor* getOrCreateRequestor( const std::string& endpoint );
    
    void onRequestorDestroyed( const std::string& endpoint );
    
    void broadcastBarrierUp();
    
    void broadcastBarrierDown();
    
    inline InstanceManager* getInstanceManager() { return _instanceMan; }
private:
    InstanceManager* _instanceMan;
    ITransport* _transport;
    ServerRequestHandler* _srh;
    std::string _publicEndpoint;
    std::map<std::string, Requestor*> _requestors;
};

}
}

#endif