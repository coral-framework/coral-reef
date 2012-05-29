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
    
    co::int32 destInstanceId;
    Demarshaller::MsgType type;
    bool hasReturn;
    _demarshaller.setMarshalledRequest( msg, type, destInstanceId, hasReturn );
    
    Invoker* invoker = _lcm->getInvoker( destInstanceId );
    std::string returned;
    invoker->invoke( _demarshaller, hasReturn, returned );
    
    if( hasReturn )
        _link->sendReply( returned );    
}


}
}