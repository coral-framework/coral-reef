#include "ServerNode_Base.h"
#include <map>

#include "Decoder.h"
#include "network/Connection.h"
#include <stack>

namespace reef {

class Channel;
class Servant;
    
class ServerNode : public ServerNode_Base
{
public:
    ServerNode();
    
    virtual ~ServerNode();
    
    void start( const std::string& address );
    
	void update();

    void stop();
       
    // DecoderChannel
    int newInstance( const std::string& typeName );

	void removeInstance( co::int32 instanceId );
    
private:
	Binder _binder;
    Decoder _decoder;

    typedef std::vector<Channel*> Channels;
    
    Channels _channels;
    
    std::stack<co::int32> _freedIds;
};
    
} // namespace reef