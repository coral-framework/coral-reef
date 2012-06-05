#ifndef _REEF_CLIENTREQUESTHANDLER_H_
#define _REEF_CLIENTREQUESTHANDLER_H_

#include <string>

namespace reef {
namespace rpc {
    
class Node;
class IActiveLink;
    
class ClientRequestHandler
{
public:
    ClientRequestHandler( IActiveLink* link, Node* node );
    
    // Low level API used by the ClientProxies
    void handleAsynchRequest( const std::string& request );
    
    void handleSynchRequest( const std::string& request, std::string& ret );
    
    inline const std::string& getEndpoint(){ return _endpoint; }
    
private:
    Node* _node;
    IActiveLink* _link;
    std::string _endpoint;
};

}
}

#endif