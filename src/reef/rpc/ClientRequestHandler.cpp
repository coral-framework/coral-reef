#include "ClientRequestHandler.h"

#include "Node.h"

#include <reef/rpc/IActiveLink.h>

namespace reef {
namespace rpc {

ClientRequestHandler::ClientRequestHandler( IActiveLink* link, Node* node ) : _node( node ), 
                                            _link( link ), _endpoint( link->getAddress() )
{}

// Low level API used by the ClientProxies
void ClientRequestHandler::handleAsynchRequest( const std::string& request )
{
    _link->send( request );
}

void ClientRequestHandler::handleSynchRequest( const std::string& request, std::string& ret )
{
    _link->send( request );
    
    // The Wait for the reply still keeps updating the server
    while( !_link->receiveReply( ret ) )
        _node->update();
}

}
}