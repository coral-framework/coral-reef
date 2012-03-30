#ifndef _REEF_CONNECTION_H_
#define _REEF_CONNECTION_H_

#include <string>
#include <zmq.hpp>

#include <co/reserved/RefCounted.h>

namespace reef
{
    
class Connecter : public co::RefCounted
{
public:
    // Factory method. The returned Pointer must be saved in a RefPtr.
    static Connecter* getOrOpenConnection( const std::string& address );
    ~Connecter();

	bool connect( const std::string& address );
	void close();
    
    bool isConnected() { return _connected; }

	void send( const std::string& data );
	bool receiveReply( std::string& data ); //Blocking function

    inline const std::string& getAddress() { return _address; }
private:

    Connecter();
    
	std::string _address;
	zmq::context_t _context;
    zmq::socket_t _socket;
    
    bool _connected;
};

class Binder
{
public:
	Binder();
	~Binder();

	bool bind( const std::string& address );
	void close();
    
    bool isBound() { return _bound; }

	bool receive( std::string& data ); // Non-Blocking
	void reply( const std::string& data );

private:
	std::string _address;
	zmq::context_t _context;
    zmq::socket_t _socket;

	zmq::message_t _lastSender;
    
    bool _bound;
};

} // namespace reef

#endif