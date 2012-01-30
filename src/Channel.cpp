#include "Channel.h"

Channel::Channel( int instanceId )
{
    _instanceId = instanceId;
}

Channel::~Channel()
{
    // empty
}

void Channel::setConnection( Connection* connection )
{
    _connection = connection;
}

void Channel::sendCall( co::int32 instanceId, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::call( co::int32 instanceId, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::getField( co::int32 instanceId, co::int32 serviceId, co::IField* field, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void Channel::setField( co::int32 instanceId, co::int32 serviceId, co::IField* field, const co::Any& value )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}