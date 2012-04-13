#ifndef _REEF_ZMQTRANSPORT_H_
#define _REEF_ZMQTRANSPORT_H_

#include "Transport.h"

namespace reef
{
    
class ZMQTransport : public Transport
{
protected:
    Connecter* createConnecter( const std::string& addressToConnect );
    Binder* createBinder( const std::string& addressBoundTo );
};

} // namespace reef

#endif