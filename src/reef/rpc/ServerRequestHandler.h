#ifndef _REEF_SERVEREQUESTHANDLER_H_
#define _REEF_SERVEREQUESTHANDLER_H_

#include "Demarshaller.h"

#include <reef/rpc/IPassiveLink.h>

#include <co/RefPtr.h>
#include <co/IObject.h>

namespace reef {
namespace rpc {
    
class Invoker;
    
class ServerRequestHandler
{
public:
    ServerRequestHandler( IPassiveLink* link, const std::string& publicEndpoint );
    
    ~ServerRequestHandler();
    
	void react();
    
    void reply( const std::string& reply );
    
    inline void setInvoker( Invoker* invoker ){ _invoker = invoker; }
    inline const std::string& getPublicEndpoint(){ return _publicEndpoint; }
    
private:
    co::RefPtr<IPassiveLink> _link;
    Invoker* _invoker;
    std::string _publicEndpoint;
};

}
}

#endif
