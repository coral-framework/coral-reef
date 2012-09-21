#ifndef __RPC_ZMQPASSIVELINK_H__
#define __RPC_ZMQPASSIVELINK_H__

#include "ZMQPassiveLink_Base.h"

#include <zmq.hpp>

namespace zmq {

class ZMQPassiveLink : public ZMQPassiveLink_Base
{
public:    
	ZMQPassiveLink( zmq::context_t& context );

    ZMQPassiveLink();
    
	virtual ~ZMQPassiveLink();
    
    bool bind( const std::string& address );

	// ------ rpc.IPassiveLink Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receive( std::string& msg );

	void sendReply( const std::string& msg );

private:
    std::string _address;
    
    zmq::socket_t _socket;
    
    zmq::message_t _lastSender;
};

} // namespace zmq

#endif