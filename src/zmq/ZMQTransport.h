#ifndef __REEF_ZMQTRANSPORT_H__
#define __REEF_ZMQTRANSPORT_H__

#include <reef/IActiveLink.h>
#include <reef/IPassiveLink.h>

#include "ZMQTransport_Base.h"

#include <map>

namespace zmq {
    
class ZMQTransport : public ZMQTransport_Base
{
public:
    ZMQTransport();
    
    virtual ~ZMQTransport();
    
    // ------ reef.ITransport Methods ------ //
    
    reef::IPassiveLink* bind( const std::string& addressToListen );
    
    reef::IActiveLink* connect( const std::string& addressToConnect );
    
    // ------ C++ only Methods ------ //
    
    void onLinkDestructor( const std::string& address );
    
private:
    reef::IActiveLink* createActiveLink( const std::string& address );
    
private:
    typedef std::map<std::string, reef::IActiveLink*> ActiveLinks;
    ActiveLinks _activeLinks;
};
    
} // namespace reef

#endif
