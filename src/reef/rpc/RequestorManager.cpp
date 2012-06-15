#include "RequestorManager.h"

#include "Requestor.h"
#include "ClientRequestHandler.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IActiveLink.h>

namespace reef {
namespace rpc {

RequestorManager::RequestorManager( Node* node, ITransport* transport, const std::string& localEndpoint )
    : _node( node ), _transport( transport ), _localEndpoint( localEndpoint )
{
}
  
RequestorManager::~RequestorManager()
{
    std::map<std::string, Requestor*>::iterator it = _requestors.begin();
    
    for( ; it != _requestors.end(); it++ )
    {
        delete it->second;
    }
}
    
Requestor* RequestorManager::getOrCreateRequestor( const std::string& endpoint )
{
    assert( _transport );
    
    std::map<std::string, Requestor*>::iterator it = _requestors.find( endpoint );
    if( it != _requestors.end() )
        return it->second;
    
    IActiveLink* link = _transport->connect( endpoint );
    ClientRequestHandler* crh = new ClientRequestHandler( link, _node );
    
    Requestor* req = new Requestor( this, crh, _localEndpoint );
    
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