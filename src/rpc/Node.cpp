#include "Node.h"

#include "Invoker.h"
#include "Requestor.h"
#include "BarrierManager.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "InstanceContainer.h"
#include "ServerRequestHandler.h"

#include <rpc/IConnector.h>
#include <rpc/IAcceptor.h>
#include <co/Exception.h>

#include <iostream>
#include <cassert>
#include <map>
#include <set>

namespace rpc {
    
Node::Node() : _srh( 0 )
{
}
    
Node::~Node()
{
    if( !_publicEndpoint.empty() ) // Node needs to be stopped
        stop();
}

const std::string& Node::getPublicAddress()
{
	return _publicEndpoint;
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
    
void Node::raiseBarrier( co::int32 capacity )
{
    assert( _srh );
    
    _barrierMan->raiseBarrier( capacity );
    
    while( _barrierMan->isBarrierUp() )
        update();
}
 
void Node::hitBarrier()
{
    _invoker->hitBarrier();
}
    
void Node::start( const std::string&  boundAddress )
{
    _publicEndpoint = boundAddress;
    
    _srh = new ServerRequestHandler( _transport->bind( boundAddress ), _publicEndpoint );

    _instanceMan = new InstanceManager();
 
    _requestorMan = new RequestorManager( _instanceMan, _transport, _srh );
    
    _barrierMan = new BarrierManager( _requestorMan );
    
    _invoker = new Invoker( _instanceMan, _barrierMan, _srh, _requestorMan );
    _srh->setInvoker( _invoker );
}
    
void Node::update()
{
    assert( _srh );
        
	_srh->react();
}

void Node::stop()
{
    assert( !_publicEndpoint.empty() );
    
    _srh->setInvoker( 0 );
    delete _invoker; _invoker = 0;
    delete _barrierMan;
    delete _requestorMan; _requestorMan = 0;
    delete _instanceMan; _instanceMan = 0;
    delete _srh; _srh = 0;

	_publicEndpoint.clear();
}

co::IObject* Node::getInstance( co::int32 instanceId )
{
    return _instanceMan->getInstance( instanceId )->getInstance();
}
 
co::int32 Node::publishInstance( co::IObject* instance, const std::string& key )
{
    return _instanceMan->publishInstance( instance, key );
}
    
void Node::unpublishInstance( const std::string& key )
{
    _instanceMan->unpublishInstance( key );
}
        
rpc::ITransport* Node::getTransportService()
{
    return _transport;
}

void Node::setTransportService( rpc::ITransport* transport )
{
    _transport = transport;
}
        
CORAL_EXPORT_COMPONENT( Node, Node );
    
} // namespace rpc