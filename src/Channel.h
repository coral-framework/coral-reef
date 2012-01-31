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
    Channel( Connection* connection );
    ~Channel();
    
    void setConnection( Connection* connection );
    
    /*
        Establishes a new communication channel to a new remote instance specified by 
        \a remoteTypeName. A new instace of \a remoteTypeName will be created at 
        server side and bounded to this channel. The unique id of the remote instance 
        will be retrieved.
     
        This method makes this channel permanently bound to the new remote instance
        and retrieves -1 if it had already been previously established.
     */
    int establish( const std::string& remoteTypeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
private:
    int _remoteInstanceId;
    Connection* _connection;
};

} // namespace reef

#endif
