#ifndef __RPC_NODE_H__
#define __RPC_NODE_H__

#include "Node_Base.h"
#include "Timer.h"

#include "Invoker.h"
#include "Demarshaller.h"
#include "Marshaller.h"

#include <rpc/ITransport.h>
#include <rpc/IAcceptor.h>

#include <map>
#include <stack>

namespace rpc {

class Client;
class Invoker;
class BarrierManager;
class InstanceManager;
class RequestorManager;
class ServerRequestHandler;
    
class Node : public Node_Base
{
public:    
    Node();
    
    virtual ~Node();
    
    // INode methods

	std::string getPublicAddress();

    co::IObject* newRemoteInstance( const std::string& instanceType, const std::string& address );
     
    co::IObject* getRemoteInstance( const std::string& instanceType, const std::string& key, 
                                    const std::string& address );

	void discoverRemoteInstances(const std::string& componentTypeName, const std::string& key, 
		co::uint32 timeout, std::vector<co::IObjectRef>& instances, std::vector<rpc::INetworkNodeRef>& instancesInfo);

    void raiseBarrier( co::int32 capacity, co::uint32 timeout );
    void hitBarrier( co::uint32 timeout );
    
    void start( const std::string& boundAddress );
    
	void update();

	void setInvokingTimeout( co:: int32 seconds );
	
    void stop();

	co::IObject* getInstance( co::int32 instanceId );
    
	co::int32 publishInstance( co::IObject* instance, const std::string& key );
    
    void unpublishInstance( const std::string& key );
    
protected: // receptacles
    rpc::ITransport* getTransportService();	
    
	void setTransportService( rpc::ITransport* transport );

public:
    
    // C++ only methods
    inline const std::string& getPublicEndpoint(){ assert( !_publicEndpoint.empty() ); 
        return _publicEndpoint; }
    
    
private:
	double _elapsedSinceLastAutoDiscoverySignal;
	bool _started;
	bool _autoDiscovery;
    RequestorManager* _requestorMan;
    
	Timer _timer;
    InstanceManager* _instanceMan;
    BarrierManager* _barrierMan;
    Invoker* _invoker;
    ServerRequestHandler* _srh;
    
    ITransport* _transport;
        
    std::string _publicEndpoint;
};
    
} // namespace rpc

#endif