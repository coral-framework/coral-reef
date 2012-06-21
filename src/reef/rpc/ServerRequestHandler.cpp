#include "ServerRequestHandler.h"

#include "Invoker.h"

#include <co/Any.h>

namespace reef {
namespace rpc {

ServerRequestHandler::ServerRequestHandler( IPassiveLink* link ) : _link( link )
{
    assert( _link.get() );
}

ServerRequestHandler::~ServerRequestHandler()
{
}
    
void ServerRequestHandler::react()
{
    std::string msg;
    if( !_link->receive( msg ) )
        return;
    
    _invoker->dispatchInvocation( msg );
}

void ServerRequestHandler::reply( const std::string& reply )
{
    _link->sendReply( reply );
}
    

}
}