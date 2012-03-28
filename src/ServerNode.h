#include "ServerNode_Base.h"
#include <map>

#include "Decoder.h"
#include "network/Connection.h"
#include <stack>

namespace reef {

class Servant;
class Servant;
    
class ServerNode : public ServerNode_Base
{
public:
    ServerNode();
    
    virtual ~ServerNode();
    
    void start( const std::string& address );
    
	void update();

    void stop();
       
    co::int32 newInstance( const std::string& typeName );

    /*
     Makes an instance available for remote usage. Which means: creating a 
    Virtual Address, and start a remote-references counting for it.
     Only increment reference and return address if already available.
     */
    co::int32 openRemoteReference( co::IObject* instance );
    
    /*
     Remove a remote reference to the instance. If there are no more references,
     remove the internal reference to it and cleear the virtual address.
     */
    void closeRemoteReference( co::int32 vAddress );
    
private:
    void releaseInstance( co::int32 vAddress );
    
     // Creates a servant for the instance and returns its VA
    co::int32 publishInstance( co::IObject* instance );
    
    co::int32 newVirtualAddress();
    
    // returns -1 if not found
    co::int32 getVirtualAddress( const co::IObject* instance );
    
private:
	Binder _binder;
    Decoder _decoder;

    // the servants and the number of remote references for it
    std::vector<Servant*> _servants;
    std::vector<co::int32> _remoteRefCounting;
    
    // used to map easily an instance to a virtual address
    typedef std::pair<co::IObject*, co::int32> objToAddress;
    typedef std::map<const co::IObject*, co::int32> VirtualAddresses;
    VirtualAddresses _vas;

    
    std::stack<co::int32> _freedIds;
};
    
} // namespace reef