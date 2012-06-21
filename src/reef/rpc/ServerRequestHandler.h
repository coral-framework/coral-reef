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
    ServerRequestHandler( IPassiveLink* link );
    
    ~ServerRequestHandler();
    
    inline void setInvoker( Invoker* invoker ){ _invoker = invoker; }
    
	void react();
    
    void reply( const std::string& reply );
    
private:
    co::RefPtr<IPassiveLink> _link;
    Invoker* _invoker;
};

}
}

#endif
