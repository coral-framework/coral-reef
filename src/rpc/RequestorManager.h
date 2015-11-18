#ifndef _RPC_REQUESTORMANAGER_H_
#define _RPC_REQUESTORMANAGER_H_

#include <map>
#include <string>

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

	void setClientRequestTimeout( int seconds );
    
    inline InstanceManager* getInstanceManager() { return _instanceMan; }
private:	
    InstanceManager* _instanceMan;
    ITransport* _transport;
    ServerRequestHandler* _srh;
    std::string _publicEndpoint;
    std::map<std::string, Requestor*> _requestors;
};

} // namespace rpc

#endif