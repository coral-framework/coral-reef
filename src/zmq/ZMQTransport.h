#ifndef __REEF_ZMQTRANSPORT_H__
#define __REEF_ZMQTRANSPORT_H__

#include <reef/rpc/IActiveLink.h>
#include <reef/rpc/IPassiveLink.h>

#include "ZMQTransport_Base.h"

#include <map>

namespace zmq {
    
class ZMQTransport : public ZMQTransport_Base
{
public:
    ZMQTransport();
    
    virtual ~ZMQTransport();
    
    // ------ reef.ITransport Methods ------ //
    
    reef::rpc::IPassiveLink* bind( const std::string& addressToListen );
    
    reef::rpc::IActiveLink* connect( const std::string& addressToConnect );
    
    // ------ C++ only Methods ------ //
    
    void onLinkDestructor( const std::string& address );
    
private:
    reef::rpc::IActiveLink* createActiveLink( const std::string& address );
    
private:
    typedef std::map<std::string, reef::rpc::IActiveLink*> ActiveLinks;
    ActiveLinks _activeLinks;
};
    
} // namespace reef

#endif
