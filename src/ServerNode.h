#include "ServerNode_Base.h"
#include "Channel.h"
#include <map>

namespace reef {

class Binder;
class Channel;
    
class ServerNode : public ServerNode_Base, public OutputChannelDelegate
{
public:
    ServerNode();
    
    virtual ~ServerNode();
    
    void start( const std::string& address );
    
	void update();

    void stop();
       
    // OutputChannelDelegate
    int onNewInstance( Channel* channel, const std::string& typeName );

	void registerInstance( co::int32 virtualAddress, co::IObject* object );
    
	co::IObject* mapInstance( co::int32 virtualAddress );

private:
	Binder* _binder;

    typedef std::vector<Channel*> Channels;
    
    Channels _channels;
	typedef std::map<co::int32,co::IObject*> InstanceMap;
	InstanceMap _instanceMap;
};
    
} // namespace reef