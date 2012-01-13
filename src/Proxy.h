/*
 * Component implementation template for 'reef.Proxy'.
 */
#ifndef PROXY_H
#define PROXY_H

#include "Proxy_Base.h"
#include "Networking.h"


namespace reef {

class Node;

class Proxy : public Proxy_Base
{
public:
	Proxy();
    
    // receives the node it belongs, the index of the rmt node that has the real object, and its own id
	Proxy( reef::Node* node, co::int32 remoteNodeIndex, co::int32 proxyId );

	virtual ~Proxy();
	
    void callMethodById( co::int32 id );
    
	void callMethodByName( const std::string& name );
    
	void receiveMsg( Message* msg );
    
    inline co::int32 getRemoteNode()
    {
        return _rmtNodeId;
    }
    
    inline co::int32 getMyId()
    {
        return _myId;
    }

private:
    // the node this belongs to
	reef::Node* _node;
	
    // the index (local node's address table) of the remote node that has the real object
    co::int32 _rmtNodeId;
    
    // _myId + this node's Id will be this proxy's identifier for the remote node 
	co::int32 _myId;

};



} // namespace reef

#endif