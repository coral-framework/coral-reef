#ifndef _REEF_CHANNEL_H_
#define _REEF_CHANNEL_H_

#include <co/Any.h>
#include <co/Coral.h>

#include "Connection.h"

namespace reef 
{
    
class Channel
{
    public:
    Channel( int instanceId );
    ~Channel();
    
    void setConnection( Connection* connection );
    
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
private:
    int _instanceId;
    Connection* _connection;
};

} // namespace reef

#endif
