#ifndef __REEF_ZMQTRANSPORT_H__
#define __REEF_ZMQTRANSPORT_H__

#include "ZMQTransport_Base.h"

#include <map>

namespace reef {

class IActiveLink;
class IPassiveLink;
    
class ZMQTransport : public ZMQTransport_Base
{
public:
    ZMQTransport();
    
    virtual ~ZMQTransport();
    
    // ------ reef.ITransport Methods ------ //
    
    IPassiveLink* bind( const std::string& addressToListen );
    
    IActiveLink* connect( const std::string& addressToConnect );
    
    // ------ C++ only Methods ------ //
    
    void onLinkDestructor( const std::string& address );
    
private:
    IActiveLink* createActiveLink( const std::string& address );
    
private:
    typedef std::map<std::string, IActiveLink*> ActiveLinks;
    ActiveLinks _activeLinks;
};
    
} // namespace reef

#endif
