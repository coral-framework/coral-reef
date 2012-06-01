#include "RequestorCache.h"

#include "Requestor.h"

#include <reef/rpc/ITransport.h>
#include <reef/rpc/IActiveLink.h>

namespace reef {
namespace rpc {

RequestorCache::RequestorCache( Node* node, ITransport* transport, const std::string& localEndpoint )
    : _node( node ), _transport( transport ), _localEndpoint( localEndpoint )
{
}
    
Requestor* RequestorCache::getOrCreate( const std::string& endpoint )
{
    assert( _transport );
    
    std::map<std::string, Requestor*>::iterator it = _requestors.find( endpoint );
    if( it != _requestors.end() )
        return it->second;
    
    IActiveLink* link = _transport->connect( endpoint );
    Requestor* req = new Requestor( _node, link, _localEndpoint, this );
    
    std::pair<std::string, Requestor*> reqPair( endpoint, req );
    _requestors.insert( reqPair );
    
    return req;
}
    
void RequestorCache::onRequestorDestroyed( const std::string& endpoint )
{
    std::map<std::string, Requestor*>::iterator it = _requestors.find( endpoint );
    assert( it != _requestors.end() );
    
    _requestors.erase( it );
}
    
}
}