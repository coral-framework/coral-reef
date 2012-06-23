#include "Node.h"

#include "Invoker.h"
#include "Requestor.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "InstanceContainer.h"
#include "ServerRequestHandler.h"

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>
#include <co/Exception.h>

#include <iostream>
#include <cassert>
#include <map>
#include <set>

namespace reef {
namespace rpc {
    
Node::Node() : _srh( 0 )
{
}
    
Node::~Node()
{
    if( !_publicEndpoint.empty() ) // Node needs to be stopped
        stop();
}
    
co::IObject* Node::newRemoteInstance( const std::string& instanceType, 
                                           const std::string& address )
{
    Requestor* req = _requestorMan->getOrCreateRequestor( address );
    return req->requestNewInstance( instanceType );
}

co::IObject* Node::findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    Requestor* req = _requestorMan->getOrCreateRequestor( address );
    return req->requestPublicInstance( key, instanceType );
}
    
void Node::start( const std::string&  boundAddress, const std::string& publicAddress )
{
    _publicEndpoint = publicAddress;
    
    _srh = new ServerRequestHandler( _transport->bind( boundAddress ) );

    _instanceMan = new InstanceManager();
 
    _requestorMan = new RequestorManager( this, _instanceMan, _transport, _publicEndpoint );
    
    _invoker = new Invoker( this, _instanceMan, _srh, _requestorMan );
}
    
void Node::update()
{
    assert( _srh );
        
	_srh->react( _invoker );
}

void Node::stop()
{
    assert( !_publicEndpoint.empty() );
    
    delete _invoker; _invoker = 0;
    delete _requestorMan; _requestorMan = 0;
    delete _instanceMan; _instanceMan = 0;
    delete _srh; _srh = 0;
}

co::IObject* Node::getInstance( co::int32 instanceId )
{
    return _instanceMan->getInstance( instanceId )->getInstance();
}

co::int32 Node::getInstanceNumLeases( co::int32 instanceId )
{
    return _instanceMan->getInstanceNumLeases( instanceId );
}
 
co::int32 Node::publishInstance( co::IObject* instance, const std::string& key )
{
    return _instanceMan->publishInstance( instance, key );
}
    
void Node::unpublishInstance( const std::string& key )
{
    _instanceMan->unpublishInstance( key );
}
        
reef::rpc::ITransport* Node::getTransportService()
{
    return _transport;
}

void Node::setTransportService( reef::rpc::ITransport* transport )
{
    _transport = transport;
}
        
CORAL_EXPORT_COMPONENT( Node, Node );
    
}
    
} // namespace reef