#ifndef _RPC_SERVEREQUESTHANDLER_H_
#define _RPC_SERVEREQUESTHANDLER_H_

#include "Demarshaller.h"

#include <rpc/IAcceptor.h>

#include <co/RefPtr.h>
#include <co/IObject.h>

namespace rpc {
    
class Invoker;
    
class ServerRequestHandler
{
public:
    ServerRequestHandler( IAcceptor* link, const std::string& publicEndpoint );
    
    ~ServerRequestHandler();
    
	void react();
    
    void reply( const std::string& reply );
    
    inline void setInvoker( Invoker* invoker ){ _invoker = invoker; }
    inline const std::string& getPublicEndpoint(){ return _publicEndpoint; }
    
private:
    co::RefPtr<IAcceptor> _link;
    Invoker* _invoker;
    std::string _publicEndpoint;
};

} // namespace rpc

#endif
