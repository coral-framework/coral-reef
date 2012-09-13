#ifndef __REEF_NODE_H__
#define __REEF_NODE_H__

#include "Node_Base.h"

#include "Invoker.h"
#include "Demarshaller.h"
#include "Marshaller.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IPassiveLink.h>

#include <map>
#include <stack>

namespace reef {
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

	const std::string& getPublicAddress();

    co::IObject* newRemoteInstance( const std::string& instanceType, const std::string& address );
     
    co::IObject* findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                    const std::string& address );

    void raiseBarrier( co::int32 capacity );
    void hitBarrier();
    
    void start( const std::string& boundAddress, const std::string& publicEndpoint );
    
	void update();

    void stop();

	co::IObject* getInstance( co::int32 instanceId );
    
    co::int32 getInstanceNumLeases( co::int32 instanceId );
    
	co::int32 publishInstance( co::IObject* instance, const std::string& key );
    
    void unpublishInstance( const std::string& key );
    
protected: // receptacles
    reef::rpc::ITransport* getTransportService();
    
	void setTransportService( reef::rpc::ITransport* transport );

public:
    
    // C++ only methods
    inline const std::string& getPublicEndpoint(){ assert( !_publicEndpoint.empty() ); 
        return _publicEndpoint; }
    
    
private:
    RequestorManager* _requestorMan;
    
    InstanceManager* _instanceMan;
    BarrierManager* _barrierMan;
    Invoker* _invoker;
    ServerRequestHandler* _srh;
    
    ITransport* _transport;
        
    std::string _publicEndpoint;
};
    
}
    
} // namespace reef

#endif