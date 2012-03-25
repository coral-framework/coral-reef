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
       
    int newInstance( const std::string& typeName );

	void removeInstance( co::int32 instanceId );
    
    /*
     checks if an object has been published (made available for remote access).
     If published, increment the instance's ref counting and returns its Virtual 
     Address, else return -1.
    */
    co::int32 newRemoteAccessor( const co::IObject* instance );
    
    
    void removeRemoteAccessor( const co::IObject* instance );
    
    /*
     Makes an instance available for remote usage. Which means: creating a 
    Virtual Address, and start a remote-references counting for it.
     */
    co::int32 publishInstance( const co::IObject* instance );
    
private:
	Binder _binder;
    Decoder _decoder;

    typedef std::vector<Servant*> Channels;
    typedef std::vector<co::IObject*> Instances;
    
    typedef std::pair<co::IObject*, co::int32> objToAddress;
    typedef std::map<co::IObject*, co::int32> VirtualAddresses;
    
    Instances _instances;
    Channels _channels;
    VirtualAddresses _vas;
    
    std::stack<co::int32> _freedIds;
};
    
} // namespace reef