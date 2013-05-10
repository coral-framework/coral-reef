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
#include <ctime>
#include <sstream>

namespace rpc {
    
Node::Node() : _srh( 0 )
{
}
    
Node::~Node()
{
    if( !_publicEndpoint.empty() ) // Node needs to be stopped
        stop();
}

std::string Node::getPublicAddress()
{
	return _publicEndpoint;
}

co::IObject* Node::newRemoteInstance( const std::string& instanceType, 
                                           const std::string& address )
{
    co::RefPtr<Requestor> req = _requestorMan->getOrCreateRequestor( address );
    return req->requestNewInstance( instanceType );
}

co::IObject* Node::findRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    co::RefPtr<Requestor> req = _requestorMan->getOrCreateRequestor( address );
    return req->requestPublicInstance( key, instanceType );
}
    
void Node::raiseBarrier( co::int32 capacity, co::uint32 timeout )
{
    assert( _srh );
    
	time_t tstart = time(0);
	time_t current;

    _barrierMan->raiseBarrier( capacity );
    
    while( _barrierMan->isBarrierUp() )
	{
		current = time(0);

        if( current - tstart > timeout )
		{
			CORAL_THROW( RemotingException, "Reply receiving timeout" );
		}
        update();
	}
}
 
void Node::hitBarrier()
{
    _invoker->hitBarrier();
}
    
void Node::start( const std::string&  boundAddress )
{
    _srh = new ServerRequestHandler( _transport->bind( boundAddress ), boundAddress );

    _instanceMan = new InstanceManager();
 
    _requestorMan = new RequestorManager( _instanceMan, _transport, _srh );
    
    _barrierMan = new BarrierManager( _requestorMan );
    
    _invoker = new Invoker( _instanceMan, _barrierMan, _srh, _requestorMan );
    _srh->setInvoker( _invoker );
	_publicEndpoint = boundAddress;
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