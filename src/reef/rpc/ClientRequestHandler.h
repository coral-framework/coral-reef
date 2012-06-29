#ifndef _REEF_CLIENTREQUESTHANDLER_H_
#define _REEF_CLIENTREQUESTHANDLER_H_

#include <reef/rpc/IActiveLink.h>
#include <co/RefPtr.h>

#include <string>


namespace reef {
namespace rpc {
    
class Node;
class IActiveLink;
class ServerRequestHandler;
    
class ClientRequestHandler
{
public:
    ClientRequestHandler( IActiveLink* link, ServerRequestHandler* srh );
    
    // Low level API used by the ClientProxies
    void handleAsynchRequest( const std::string& request );
    
    void handleSynchRequest( const std::string& request, std::string& ret );
    
    inline const std::string& getEndpoint(){ return _endpoint; }
    
private:
    co::RefPtr<IActiveLink> _link;
    ServerRequestHandler* _srh;
    std::string _endpoint;
};

}
}

#endif