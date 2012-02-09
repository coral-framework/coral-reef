#ifndef _REEF_CONNECTION_H_
#define _REEF_CONNECTION_H_

#include <string>
#include <zmq.hpp>

namespace reef
{
    


class Connecter
{
public:
	Connecter();
	~Connecter();

	bool connect( const std::string& address );
	void close();

	void send( const std::string& data );
	bool receiveReply( std::string& data ); //Blocking function

private:
	std::string _address;
	zmq::context_t _context;
    zmq::socket_t _socket;
};

class Binder
{
public:
	Binder();
	~Binder();

	bool bind( const std::string& address );
	void close();

	bool receive( std::string& data ); // Non-Blocking
	void reply( const std::string& data );

private:
	std::string _address;
	zmq::context_t _context;
    zmq::socket_t _socket;

	zmq::message_t _lastSender;
};

} // namespace reef

#endif
