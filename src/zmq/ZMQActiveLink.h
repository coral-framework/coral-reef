#ifndef __RPC_ZMQACTIVELINK_H__
#define __RPC_ZMQACTIVELINK_H__

#include "ZMQConnector_Base.h"

#include <zmq.hpp>

namespace zmq {
    
class ZMQTransport;

class ZMQConnector : public ZMQConnector_Base
{
public:
    // The \a creator wil be notified upon the destruction of this object.
    ZMQConnector( zmq::context_t& context );
    
	ZMQConnector();

	virtual ~ZMQConnector();
    
    bool connect( const std::string& address );

	// ------ rpc.IConnector Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receiveReply( std::string& msg );

	void send( const std::string& msg );

private:
	std::string _address;
    
    zmq::socket_t _socket;    
};

} // namespace zmq

#endif