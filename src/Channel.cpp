#include "Channel.h"

namespace
{
    const std::string MSG_NEW_REMOTE_INSTANCE   = "nri ";
    const std::string MSG_SEND_CALL             = "sca";
    const std::string MSG_CALL                  = "mca";
    const std::string MSG_GET_FIELD             = "gfl";
}

namespace reef 
{
    
Channel::Channel( Connection* connection )
{
    setConnection( connection );
}

Channel::~Channel()
{
    // empty
}

void Channel::setConnection( Connection* connection )
{
    _connection = connection;
}
    
int Channel::establish( const std::string& remoteTypeName )
{
    _stream.clear();
    _stream << _channelId << ":" << MSG_NEW_REMOTE_INSTANCE;

    _connection->send( _stream.str() );
    
    std::string result;
    _connection->receive( result );
    
    _stream.clear();
   
    // convert to int
    _stream << result;
    _stream >> _channelId;
    
    return _channelId;
 }                       

void Channel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

} // namespace reef