#ifndef __REEF_ZMQPASSIVELINK_H__
#define __REEF_ZMQPASSIVELINK_H__

#include "ZMQPassiveLink_Base.h"

#include <zmq.hpp>

namespace reef {

class ZMQPassiveLink : public ZMQPassiveLink_Base
{
public:    
	ZMQPassiveLink();

	virtual ~ZMQPassiveLink();
    
    bool bind( const std::string& address );

	// ------ reef.IPassiveLink Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receive( std::string& msg );

	void sendReply( const std::string& msg );

private:
    std::string _address;
    zmq::context_t _context;
    zmq::socket_t _socket;
    
    zmq::message_t _lastSender;
    std::string _senderAddress;
};

} // namespace reef

#endif