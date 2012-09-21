#include "ServerRequestHandler.h"

#include "Invoker.h"

#include <co/Any.h>

namespace rpc {

ServerRequestHandler::ServerRequestHandler( IPassiveLink* link, const std::string& publicEndpoint ) 
        : _link( link ), _publicEndpoint( publicEndpoint )
{
    assert( _link.get() );
}

ServerRequestHandler::~ServerRequestHandler()
{
}
    
void ServerRequestHandler::react()
{
    assert( _invoker && _link.get() );
    
    std::string msg;
    if( !_link->receive( msg ) )
        return;
    
    _invoker->dispatchInvocation( msg );
}

void ServerRequestHandler::reply( const std::string& reply )
{
    _link->sendReply( reply );
}
    
} // namespace rpc