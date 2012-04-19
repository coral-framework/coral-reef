#ifndef __REEF_NODE_H__
#define __REEF_NODE_H__

#include "Node_Base.h"

#include "Servant.h"
#include "Decoder.h"
#include "Encoder.h"

#include <reef/ITransport.h>
#include <reef/IPassiveLink.h>

#include <map>
#include <stack>

namespace reef {
    
class Node : public Node_Base
{
public:    
    Node();
    
    virtual ~Node();
    
    // INode methods
    co::IObject* newRemoteInstance( const std::string& instanceType, const std::string& address );
    
    void start( const std::string& boundAddress, const std::string& publicAddress );
    
    void startTest( const std::string& boundAddress, const std::string& publicAddress );
    
	void update();

    void stop();
    
protected: // receptacles
    reef::ITransport* getTransportService();
    
	void setTransportService( reef::ITransport* transport );

public:
    
    // C++ only methods
    inline const std::string& getPublicAddress(){ assert( !_myPublicAddress.empty() ); 
        return _myPublicAddress; }
    
    // returns a local instance that is mapped to the provided ID
    inline co::IObject* getInstanceFor( co::int32 instanceID )
    { return getServantFor( instanceID )->getObject(); }
    
    /*
     Makes an instance available for remote usage. Which means: creating a 
    Virtual Address, and start a remote-references counting for it.
      If already available, then only increment reference and return address.
     */
    co::int32 publishInstance( co::IObject* instance );
    
    // Create a new remoteObject pointing to the instance or fetch a existing one
    co::IObject* getRemoteInstance( const std::string& instanceType, co::int32 instanceID, 
                                          const std::string& ownerAddress );
    
private:
    // Sends a message to another node to create an instance of provided type, blocking.
    co::int32 requestNewInstance( IActiveLink* link, const std::string& componentName );
    
    // Dispatches the msg received during an update()
    void dispatchMessage( const std::string& msg );
    
    // If a message is destined to the own Node it will be dispatched to these methods.
    void onNewInstMsg(); // creates instance, start its ref counting and replies the instanceID
    void onAccessInstMsg(); // increments the instance ref count
    
    void onMsgForServant( co::int32 instanceID, bool hasReturn );
       
private:
    // Add a new reference for an instance. TODO: set referer
    void openRemoteReference( co::int32 instanceID );
    
    /*
     Remove a remote reference to the instance. If there are no more references,
     remove the internal reference to it and cleear the virtual address.
     */
    void closeRemoteReference( co::int32 instanceID ); // TODO clear referer
    
    // Creates a servant for the instance and returns its new instanceID
    co::int32 startRemoteRefCount( co::IObject* instance );

    void releaseInstance( co::int32 instanceID );
    
    co::int32 newVirtualAddress();
    
    Servant* getServantFor( co::int32 instanceID );
    
    // returns -1 if not found
    co::int32 getInstanceID( const co::IObject* instance );
    
private:
    ITransport* _transport;
    co::RefPtr<IPassiveLink> _passiveLink;
    Decoder _decoder;
    Encoder _encoder;
    
    std::string _myPublicAddress;

    // the servants and the number of remote references for it
    std::vector<Servant*> _servants;
    std::vector<co::int32> _remoteRefCounting;
    // TODO: map referers to servants, so they can be queried for maintaining the references
    // TODO: remember to set the referer in every place which increment _remoteRefCounting 
    
    // used to map easily an instance to a virtual address
    typedef std::pair<co::IObject*, co::int32> objToAddress;
    typedef std::map<const co::IObject*, co::int32> VirtualAddresses;
    VirtualAddresses _vas;

    
    std::stack<co::int32> _freedIds;
};
    
} // namespace reef

#endif