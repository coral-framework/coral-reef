#ifndef __REEF_ZMQTRANSPORT_H__
#define __REEF_ZMQTRANSPORT_H__

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>

#include "ZMQTransport_Base.h"

#include <zmq.hpp>
#include <map>

namespace zmq {
    
class ZMQTransport : public ZMQTransport_Base
{
public:
    ZMQTransport();
    
    virtual ~ZMQTransport();
    
    // ------ reef.ITransport Methods ------ //
    
    reef::rpc::IPassiveLink* bind( const std::string& addressToListen );
    
    reef::rpc::IActiveLink* connect( const std::string& addressToConnect );

private:
    zmq::context_t _context;
};
    
} // namespace reef

#endif
