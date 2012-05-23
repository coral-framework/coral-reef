#ifndef __REEF_NODE_H__
#define __REEF_NODE_H__

#include "Node_Base.h"

#include "Invoker.h"
#include "Unmarshaller.h"
#include "Marshaller.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IPassiveLink.h>

#include <map>
#include <stack>

namespace reef {
namespace rpc {

class Client;
    
class Node : public Node_Base
{
public:    
    Node();
    
    virtual ~Node();
    
    // INode methods
    co::IObject* newRemoteInstance( const std::string& instanceType, const std::string& address );
     
    co::IObject* findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                    const std::string& address );

    void start( const std::string& boundAddress, const std::string& publicAddress );
    
	void update();

    void stop();
       
	co::IObject* getInstance( co::int32 instanceId );
    
    co::int32 getRemoteReferences( co::int32 instanceId );
    
	co::int32 publishInstance( co::IObject* instance, const std::string& key );
    
    void unpublishInstance( const std::string& key );
    
protected: // receptacles
    reef::rpc::ITransport* getTransportService();
    
	void setTransportService( reef::rpc::ITransport* transport );

public:
    
    // C++ only methods
    inline const std::string& getPublicAddress(){ assert( !_myPublicAddress.empty() ); 
        return _myPublicAddress; }
    
    /*
     Makes an instance available for remote usage but only through its ID. Only used when the
     instance has been sent as a parameter to another host.
     */
    co::int32 publishAnonymousInstance( co::IObject* instance );
    
    // Informs the instance owner about a new access to the instance (increase instance's ref count)
    void requestBeginAccess( const std::string& address, co::int32 instanceId,
                               const std::string& referer );
    
    // Informs the instance owner about a new access to the instance (increase instance's ref count)
    void requestEndAccess( reef::rpc::IActiveLink* link, co::int32 instanceId,
                            const std::string& referer );
    
    // returns a proxy to the requested remote instance
    co::IObject* getRemoteInstance( const std::string& instanceType, co::int32 instanceId, 
                                          const std::string& ownerAddress );
    
private:
    // Sends a message to another node to create an instance of provided type, blocking.
    co::int32 requestNewInstance( IActiveLink* link, const std::string& componentName );
    
    // Sends a message to another node to search for an instance published under "key", blocking.
    co::int32 requestFindInstance( IActiveLink* link, const std::string& key );
    
    // Dispatches the msg received during an update()
    void dispatchMessage( const std::string& msg );
    
    // If a message is destined to the own Node it will be dispatched to these methods.
    void onNewInstMsg(); // creates instance, start its ref counting and replies the instanceId
    void onAccessInstMsg(); // increments the instance ref count
    void onFindInstMsg(); // finds an instance published under a key, increment ref and return ID
    
    void onMsgForInvoker( co::int32 instanceId, bool hasReturn );
       
private:
    // Add a new reference for an instance. TODO: set referer
    void openRemoteReference( co::int32 instanceId );
    
    /*
     Remove a remote reference to the instance. If there are no more references,
     remove the internal reference to it and cleear the virtual address.
     */
    void closeRemoteReference( co::int32 instanceId ); // TODO clear referer
    
    // Creates a invoker for the instance and returns its new instanceId
    co::int32 startRemoteRefCount( co::IObject* instance );

    void releaseInstance( co::int32 instanceId );
    
    co::int32 newVirtualAddress();
    
    Invoker* getInvokerFor( co::int32 instanceId );
    
    // returns -1 if not found
    co::int32 getinstanceId( const co::IObject* instance );
    
    //! returns the Client indexed by \param ip or null if there is not client indexed.
    Client* searchReferer( const std::string& ip );
    
    void addReferer( const std::string& ip, Client* client );
    
private:
    ITransport* _transport;
    co::RefPtr<IPassiveLink> _passiveLink;
    Unmarshaller _unmarshaller;
    Marshaller _marshaller;
    
    std::string _myPublicAddress;

    // the invokers and the number of remote references for it
    std::vector<Invoker*> _invokers;
    std::vector<co::int32> _remoteRefCounting;
    
    std::map<std::string, Client*> _referers; //!< maps a ip to a client and its references
    
    // used to map easily an instance to a virtual address
    typedef std::pair<co::IObject*, co::int32> objToAddress;
    typedef std::map<const co::IObject*, co::int32> VirtualAddresses;
    VirtualAddresses _vas;
    
    // Instances that have been published not anonymously (through a key) will have their IDs here
    std::map<std::string, co::int32> _publicInstances;
    
    std::stack<co::int32> _freedIds; //!< auxiliary array for recicling deleted Ids
};
    
}
    
} // namespace reef

#endif