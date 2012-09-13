#include "RequestorManager.h"

#include "Requestor.h"
#include "InstanceManager.h"
#include "ClientRequestHandler.h"
#include "ServerRequestHandler.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IActiveLink.h>

namespace reef {
namespace rpc {

RequestorManager::RequestorManager( InstanceManager* instanceMan, ITransport* transp,
        ServerRequestHandler* srh ) : _instanceMan( instanceMan ),  _transport( transp ), 
            _srh( srh ), _publicEndpoint( srh->getPublicEndpoint() )
{
}
  
RequestorManager::~RequestorManager()
{
    std::map<std::string, Requestor*>::iterator it = _requestors.begin();
    
    for( ; it != _requestors.end(); it++ )
    {
        it->second->disconnect();
    }
}

void RequestorManager::broadcastBarrierUp()
{
    std::map<std::string, Requestor*>::iterator it = _requestors.begin();
    for( ; it != _requestors.end(); it++ )
    {
        it->second->requestBarrierUp();
    }
}
   
void RequestorManager::broadcastBarrierDown()
{
    std::map<std::string, Requestor*>::iterator it = _requestors.begin();
    for( ; it != _requestors.end(); it++ )
    {
        it->second->requestBarrierDown();
    }
}
    
Requestor* RequestorManager::getOrCreateRequestor( const std::string& endpoint )
{
    assert( _transport );
    
    std::map<std::string, Requestor*>::iterator it = _requestors.find( endpoint );
    if( it != _requestors.end() )
        return it->second;
    
    IActiveLink* link = _transport->connect( endpoint );
    ClientRequestHandler* crh = new ClientRequestHandler( link, _srh );
    
    Requestor* req = new Requestor( this, crh, _publicEndpoint );
    
    std::pair<std::string, Requestor*> reqPair( endpoint, req );
    _requestors.insert( reqPair );
    
    return req;
}
    
void RequestorManager::onRequestorDestroyed( const std::string& endpoint )
{
    std::map<std::string, Requestor*>::iterator it = _requestors.find( endpoint );
    assert( it != _requestors.end() );
    
    _requestors.erase( it );
}
    
}
}