#ifndef __RPC_ZMQTRANSPORT_H__
#define __RPC_ZMQTRANSPORT_H__

#include <rpc/IConnector.h>
#include <rpc/IAcceptor.h>

#include "ZMQTransport_Base.h"

#include <zmq.hpp>
#include <map>

namespace zmq {
    
class ZMQTransport : public ZMQTransport_Base
{
public:
    ZMQTransport();
    
    virtual ~ZMQTransport();
    
    // ------ rpc.ITransport Methods ------ //
    
    rpc::IAcceptor* bind( const std::string& addressToListen );
    
    rpc::IConnector* connect( const std::string& addressToConnect );

	bool sendAutoDiscoverSignal( const std::string& ipmask, int port );

	bool discoverRemoteInstances( std::vector<rpc::INetworkNodeRef>& instances, co::uint32 timeout );

	void getIpAddresses( std::vector<std::string>& addresses );

private:
	void initWinSock();

private:
    zmq::context_t _context;
	static bool _s_winsockInitialized;
};
    
} // namespace zmq

#endif
