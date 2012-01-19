/*
 * Component implementation template for 'reef.Proxy'.
 */

#include "Proxy.h"
#include "Node.h"
#include "Networking.h"

namespace reef {

Proxy::Proxy()
{
    // empty constructor
}

Proxy::Proxy( reef::Node* node, co::int32 remoteNodeIndex, co::int32 proxyId ) :
     _node( node ), _rmtNodeId( remoteNodeIndex ),  _myLocalId( proxyId )
{
}

Proxy::~Proxy()
{
    if( _node ) 
    {
        _node->onProxyDestruction( _rmtNodeId, _myLocalId );
    }
}

void Proxy::callMethodById( co::int32 id )
{
    _node->callAsynchMethod( _rmtNodeId, _myLocalId, id, co::Range<const co::Any>() );
}

void Proxy::receiveMsg( Message* msg )
{
}

CORAL_EXPORT_COMPONENT( Proxy, Proxy );
    
} // namespace reef
