#include "Node.h"

#include "Invoker.h"
#include "Requestor.h"
#include "BarrierManager.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "InstanceContainer.h"
#include "ServerRequestHandler.h"

#include <rpc/INetworkNode.h>
#include <rpc/IConnector.h>
#include <rpc/IAcceptor.h>
#include <co/Exception.h>

#include <map>
#include <set>
#include <ctime>
#include <cassert>
#include <sstream>
#include <iostream>

namespace rpc {

const double AUTO_DISCOVERY_RESEND_SIGNAL_INTERVAL = 5;
const std::string AUTO_DISCOVERY_LISTEN_PORT = ":8955";
const std::string AUTO_DISCOVERY_TALK_PORT = ":8956";

Node::Node() : _srh(0), _autoDiscovery(false), _started( false ),_elapsedSinceLastAutoDiscoverySignal(0)
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

co::IObject* Node::getRemoteInstance( const std::string& instanceType, const std::string& key, 
                                      const std::string& address )
{
    co::RefPtr<Requestor> req = _requestorMan->getOrCreateRequestor( address );
    return req->requestPublicInstance( key, instanceType );
}

void Node::discoverRemoteInstances(const std::string& componentTypeName, const std::string& key,
	co::uint32 timeout, std::vector<co::IObjectRef>& instances, std::vector<rpc::INetworkNodeRef>& instancesInfo)
{
	if (!_started)
	{
		std::vector<std::string> ips;
		_transport->getIpAddresses(ips);
		start( "tcp://" + ips[0] + AUTO_DISCOVERY_TALK_PORT );
	}
	
	_transport->discoverRemoteInstances( instancesInfo, timeout );
	
	// get discoverd instances and connect to it
	for (int i = 0; i < instancesInfo.size(); ++i)
	{		
		auto* remoteInstance = getRemoteInstance(componentTypeName, key, "tcp://" + instancesInfo[i]->getAddress() + AUTO_DISCOVERY_LISTEN_PORT);
		if (remoteInstance)
			instances.push_back(remoteInstance);
	}
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
 
void Node::hitBarrier( co::uint32 timeout )
{
    _invoker->hitBarrier( timeout );
}

void Node::start( const std::string&  boundAddress )
{
	std::string finalAddr = boundAddress;
	if (boundAddress == "autodiscover")
	{
		_autoDiscovery = true;
		std::vector<std::string> ips;
		_transport->getIpAddresses( ips );
		finalAddr = "tcp://" + ips[0] + AUTO_DISCOVERY_LISTEN_PORT;
	}
	else
		_autoDiscovery = false;

	_srh = new ServerRequestHandler(_transport->bind(finalAddr), finalAddr);

    _instanceMan = new InstanceManager();
 
    _requestorMan = new RequestorManager( _instanceMan, _transport, _srh );
    
    _barrierMan = new BarrierManager( _requestorMan );
    
    _invoker = new Invoker( _instanceMan, _barrierMan, _srh, _requestorMan );
    _srh->setInvoker( _invoker );
	_publicEndpoint = finalAddr;
	_started = true;
}
    
void Node::update()
{
    assert( _srh );
       
	_srh->react();
}

void Node::setInvokingTimeout( co::int32 seconds )
{
	if( _requestorMan )
		_requestorMan->setClientRequestTimeout( seconds );
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