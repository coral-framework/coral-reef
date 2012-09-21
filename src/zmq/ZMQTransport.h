#ifndef __RPC_ZMQTRANSPORT_H__
#define __RPC_ZMQTRANSPORT_H__

#include <rpc/IActiveLink.h>
#include <rpc/IPassiveLink.h>

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
    
    rpc::IPassiveLink* bind( const std::string& addressToListen );
    
    rpc::IActiveLink* connect( const std::string& addressToConnect );

private:
    zmq::context_t _context;
};
    
} // namespace zmq

#endif
