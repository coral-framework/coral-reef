/*
 * Component implementation template for 'reef.Node'.
 */
#ifndef NODE_H
#define NODE_H

#include "Node_Base.h"
#include <co/RefVector.h>
#include <co/Any.h>
#include <map>

namespace co {
class IComponent;
}

namespace reef {

class IProxy;
class Proxy;
class Servant;
struct Message;

class Node : public Node_Base
{
public:
	Node();

	virtual ~Node();

	// ------ reef.INode Methods ------ //

	reef::IProxy* createObject( co::int32 rmtNodeId, const std::string& componentName, co::int32 facetId );
    
    void processMsgs( co::int32 numMsgsToProcess );

    
    // ------ non Coral methods ------- //
    
    // Callback for notification of proxy being destroyed
    void onProxyDestruction( co::int32 rmtNodeId, co::int32 proxyId );
    
    // ---- Methods that the proxy use to make requests for the remote servant to answer ---- //
    
    // TODO: setValueFIeld, setServiceAt, setRefField
    void requestValueField( co::int32 rmtNodeId, co::int32 proxyId, co::int32 fieldId, co::Any& returnValue );
    
    void requestRefField( co::int32 rmtNodeId, co::int32 proxyId, co::int32 fieldId, co::IService*& returnValue );
    
    void callSynchMethod( co::int32 rmtNodeId, co::int32 proxyId, co::int32 methodId, co::Range<co::Any const> args, co::Any& returnValue );
    
    void callAsynchMethod( co::int32 rmtNodeId, co::int32 proxyId, co::int32 methodId, co::Range<co::Any const> args );
    
    // ------ Methods for the Service to answer/return the requests ------ //
    
    void sendValueType( co::int32 rmtNodeId, co::int32 rmtProxyId, co::Any value );
    
    
private:
	// member variables
	std::vector<Proxy*> _proxies;
	std::vector<Servant*> _servants;
    
    struct ProxyFullId
    {
        co::int32 rmtNodeId;
        co::int32 proxyId;
        
        ProxyFullId( co::int32 rmtNodeIdParam, co::int32 proxyIdParam ) : 
            rmtNodeId( rmtNodeIdParam ), proxyId( proxyIdParam ) {}
        
        bool operator < ( const ProxyFullId& other ) const
        {
            if( rmtNodeId == other.rmtNodeId )
                return proxyId < other.proxyId;
            else
                return rmtNodeId < other.rmtNodeId;
        }
    };
    
    typedef std::pair<ProxyFullId, co::int32> ProxyServantPair;
    
    // maps a message incoming from a remote proxy to a local servant
    std::map<ProxyFullId, co::int32> _proxyToServant; 
    
    void processMsg( Message* msg, co::int32& rmtNodeId );
};
    

} // namespace reef

#endif