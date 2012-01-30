#ifndef _REEF_CONNECTION_H_
#define _REEF_CONNECTION_H_

#include <string>

namespace reef
{
    
class Connection
{
typedef std::string Message;
    
public:
    Connection( const std::string& type, const std::string& address );
    ~Connection();
    void send( const Message& message );
    void receive( Message& message );
};

} // namespace reef

#endif
