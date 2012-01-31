#include "Channel.h"

namespace reef 
{
   
enum EventType
{
    Type_Call       = 0,
    Type_SendCall   = 1,
    Type_GetField   = 2,
    Type_SetField   = 3
};
    
static EventType extractEventType( const std::stringstream& stream, const std::string& message )
{
    // TODO: extract event type
    return Type_Call;
}
    
Channel::Channel( Connection* connection ) : _channelId( -1 )
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
    
bool Channel::handleMessage( const std::string& message )
{
    _stream.clear();
    
    _stream << message;
    
    int id;
    _stream >> id;
    if( id != _channelId )
        return false;
    
    // the message destination is this channel
    EventType type = extractEventType( _stream, message );
    switch( type )
    {
        // TODO: parameters for events
        case Type_Call:
            // call()
        case Type_SendCall:
            // sendCall()
        case Type_GetField:
            // getField()
        case Type_SetField: break;
            // setField()
    }
    
    return true;
}
    
    // Checks whether the given string is a well formed message
static bool isMessageValid( const std::string& message )    
{
    // TODO: check whether its a valid protocol message
    return true;
}

// InputChannel
InputChannel::InputChannel( Connection* connection ) : Channel( connection )
{
    // empty
}
    
InputChannel::~InputChannel()
{
    // empty
}

int InputChannel::establish( const std::string& remoteTypeName )
{
    if( _channelId != -1 )
        return -1;
    
    _stream.clear();
   // _stream << _channelId << " " << MSG_NEW_REMOTE_INSTANCE;

    _connection->send( _stream.str() );
    
    std::string result;
    _connection->receive( result );
    
    _stream.clear();
   
    // convert to int
    _stream << result;
    _stream >> _channelId;
    
    return _channelId;
}                       

void InputChannel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void InputChannel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void InputChannel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void InputChannel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}
    
// OutputChannel
    
OutputChannel::OutputChannel( Connection* connection, OutputChannelDelegate* delegate ) : Channel( connection )
{
    _delegate = delegate;
}
    
OutputChannel::~OutputChannel()
{
    // empty
}

int OutputChannel::establish( const std::string& remoteTypeName )
{
    _delegate->onChannelEstablished( this, remoteTypeName );
}

void OutputChannel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    _delegate->onSendCall( this, serviceId, method, args );
}
    
void OutputChannel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    _delegate->onCall( this, serviceId, method, args, result );
}
    
void OutputChannel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    _delegate->onGetField( this, serviceId, field, result );
}
    
void OutputChannel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    _delegate->onSetField( this, serviceId, field, value );
}

} // namespace reef