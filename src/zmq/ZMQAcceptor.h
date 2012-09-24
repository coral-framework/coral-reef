#ifndef __RPC_ZMQPASSIVELINK_H__
#define __RPC_ZMQPASSIVELINK_H__

#include "ZMQAcceptor_Base.h"

#include <zmq.hpp>

namespace zmq {

class ZMQAcceptor : public ZMQAcceptor_Base
{
public:    
	ZMQAcceptor( zmq::context_t& context );

    ZMQAcceptor();
    
	virtual ~ZMQAcceptor();
    
    bool bind( const std::string& address );

	// ------ rpc.IAcceptor Methods ------ //

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