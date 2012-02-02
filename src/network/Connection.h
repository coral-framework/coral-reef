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
        Tries to establish the connection to another connection 
        at remote server located at \a address.
        Retrieves false if this connection is aready established.
        \throws TODO: exception para problema de conexao.
     */
    bool bind( const std::string& address );
    void close();
    
    void send( const std::string& msgData );
    
    // Waits \a timeout milisseconds for a message to come.
    // Default is timeout = 0 which is infinity wait time.
    void receive( std::string& msg, int timeout = 0 );
    
    const std::string& getAddress() { return _address; }
    
private:
    std::string _address;
    zmq::context_t _context;
    zmq::socket_t _socket;
};

} // namespace reef

#endif
