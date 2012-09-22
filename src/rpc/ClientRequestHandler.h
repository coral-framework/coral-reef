#ifndef _RPC_CLIENTREQUESTHANDLER_H_
#define _RPC_CLIENTREQUESTHANDLER_H_

#include <rpc/IConnector.h>
#include <co/RefPtr.h>

#include <string>

namespace rpc {
    
class Node;
class IConnector;
class ServerRequestHandler;
    
class ClientRequestHandler
{
public:
    ClientRequestHandler( IConnector* link, ServerRequestHandler* srh );
    
    // Low level API used by the ClientProxies
    void handleAsynchRequest( const std::string& request );
    
    void handleSynchRequest( const std::string& request, std::string& ret );
    
    inline const std::string& getEndpoint(){ return _endpoint; }
    
    inline void setTimeout( co::int32 seconds ){ _timeout = seconds; }
private:
    co::RefPtr<IConnector> _link;
    ServerRequestHandler* _srh;
    std::string _endpoint;
    
    co::int32 _timeout;
};

} // namespace rpc

#endif