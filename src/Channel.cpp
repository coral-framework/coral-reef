#include "Channel.h"

enum EventType
{
    Type_NewInstance    = 0,
    Type_Call           = 1,
    Type_SendCall       = 2,
    Type_GetField       = 3,
    Type_SetField       = 4
};

static std::stringstream stream;

namespace reef 
{
    
Channel::MessageInfo Channel::getInfo( const std::string& message )
{
    MessageInfo mi;
    stream.clear();
    stream << message;
    
    stream >> mi.destination;
    stream >> mi.message;
    return mi;
}

    int Channel::getMsgDest( std::string& msg )
    {
        
    }
    
Channel::Channel() : _channelId( -1 )
{
    // empty
}

Channel::~Channel()
{
    // empty
}

// InputChannel
InputChannel::InputChannel( Connection* connection )
{   
    _connection = connection;
}
    
InputChannel::~InputChannel()
{
    // empty
}            

void InputChannel::newInstance( const std::string& typeName )
{
    return;
}

void InputChannel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    write( "1" );
}

void InputChannel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    write( "2" );
}

void InputChannel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    write( "3" );
}

void InputChannel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    write( "4" );
}
    
void InputChannel::write( const std::string& message )
{
    _connection->send( message );
}
    
// OutputChannel
OutputChannel::OutputChannel( OutputChannelDelegate* delegate )
{
    _delegate = delegate;
}
    
OutputChannel::~OutputChannel()
{
    // empty
}

void OutputChannel::newInstance( const std::string& typeName )
{
    _delegate->onNewInstance( this, typeName );
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
    
void OutputChannel::write( const std::string& rawMessage )
{
    co::Range<co::Any const> r();   
    if( rawMessage == "NI" )
        newInstance( "toto.Toto" );
    
    if( rawMessage == "CM" )
        sendCall( 0, 0, co::Range<co::Any const>() );
}

} // namespace reef