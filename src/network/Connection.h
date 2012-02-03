#ifndef _REEF_CONNECTION_H_
#define _REEF_CONNECTION_H_

#include <string>
#include <zmq.hpp>

namespace reef
{
    
class Connection
{
public:
    
    Connection( const std::string& type );
    ~Connection();
    
    /*
        TODO: document
     */
    bool bind( const std::string& address );
    
    bool connect( const std::string& address );
    
    void close();
    
    void send( const void* data, unsigned int size );
    
    // Waits \a timeout milisseconds for a message to come.
    // Default is timeout = 0 which is infinity wait time.
    void receive( void* data, unsigned int size, int timeout = 0 );
    
    const std::string& getAddress() { return _address; }
    
private:
    std::string _address;
    zmq::context_t _context;
    zmq::socket_t _socket;
};

} // namespace reef

#endif
