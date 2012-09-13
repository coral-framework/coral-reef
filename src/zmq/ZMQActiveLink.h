#ifndef __REEF_ZMQACTIVELINK_H__
#define __REEF_ZMQACTIVELINK_H__

#include "ZMQActiveLink_Base.h"

#include <zmq.hpp>

namespace zmq {
    
class ZMQTransport;

class ZMQActiveLink : public ZMQActiveLink_Base
{
public:
    // The \a creator wil be notified upon the destruction of this object.
    ZMQActiveLink( zmq::context_t& context );
    
	ZMQActiveLink();

	virtual ~ZMQActiveLink();
    
    bool connect( const std::string& address );

	// ------ reef.IActiveLink Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receiveReply( std::string& msg );

	void send( const std::string& msg );

private:
	std::string _address;
    
    zmq::socket_t _socket;    
};

} // namespace reef

#endif