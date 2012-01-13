/*
 * Component implementation template for 'reef.Node'.
 */
#include "Node.h"
#include "Networking.h"
#include "Proxy.h"
#include <reef/IProxy.h>
#include <co/IComponent.h>
#include <co/RefPtr.h>

namespace reef {

    Node::Node() : _currentChannel( 0 )
{
	// empty constructor
}

Node::~Node()
{
	// empty destructor
}

// ------ reef.INode Methods ------ //

reef::IProxy* Node::createObject( co::IComponent* type )
{
    reef::Proxy* proxy = new reef::Proxy( this, _currentChannel );
    _proxies.push_back( proxy );
    
    Message msg;
    //TODO config msg
    Network::sendMsg( 0, msg );
    
    return co::cast<reef::IProxy>( proxy->getService( "proxy" ) );
}

    void Node::onProxyDestruction( co::int32 channel )
    {
        _proxies[channel] = _proxies.back();
        _proxies.pop_back();
        
        Message msg;
        //TODO setup msg to destroy remote Servant and Service as its corresponding proxy is being destroyed
        Network::sendMsg( 0, msg );
    }
    
CORAL_EXPORT_COMPONENT( Node, Node );
    
} // namespace reef
