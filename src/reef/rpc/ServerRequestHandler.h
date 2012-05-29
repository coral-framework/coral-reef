#ifndef _REEF_SERVEREQUESTHANDLER_H_
#define _REEF_SERVEREQUESTHANDLER_H_

#include "Demarshaller.h"

#include <reef/rpc/IPassiveLink.h>

#include <co/RefPtr.h>
#include <co/IObject.h>

namespace reef {
namespace rpc {
    
class LifecycleManager;
    
class ServerRequestHandler
{
public:
    ServerRequestHandler( IPassiveLink* link );
    
    ~ServerRequestHandler();
    
	void react();
    
    // void reply( void* replyhandle, const std::string& reply );
    
private:
    Demarshaller _demarshaller;
    LifecycleManager* _lcm;
    co::RefPtr<IPassiveLink> _link;
};

}
}

#endif
