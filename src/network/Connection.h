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
    
    /* 
        Sends a byte array contained into a string.
        As data is handled as a byte array, '\0' is not a string delimiter.
     */
    void send( const std::string& data );
    
    // Waits \a timeout milisseconds for a message to come.
    // Default is timeout = 0 which is infinity wait time.
    void receive( std::string& data, int timeout = 0 );
    
    const std::string& getAddress() { return _address; }
    
private:
    std::string _address;
    zmq::context_t _context;
    zmq::socket_t* _socket;
};

} // namespace reef

#endif
