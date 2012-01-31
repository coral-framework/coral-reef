#ifndef _REEF_CONNECTION_H_
#define _REEF_CONNECTION_H_

#include <string>

namespace reef
{
    
class Connection
{
typedef std::string Message;
    
public:
    Connection( const std::string& type );
    ~Connection();
    
    /*
        Tries to establish the connection to another connection 
        at remote server located at \a address.
        Retrieves false if this connection is aready established.
        \throws TODO: exception para problema de conexao.
     */
    bool establish( const std::string& address );
    void close();
    
    void send( const Message& message );
    
    // Waits \a timeout milisseconds for a message to come.
    // Default is timeout = 0 which is infinity wait time.
    void receive( Message& message, int timeout = 0 );
    
    const std::string& getAddress() { return _address; }
    
private:
    std::string _address;
};

} // namespace reef

#endif
