#include "Channel.h"

namespace reef 
{
    
void Channel::route( std::string& msg, const std::vector<Channel*>& channels )
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>( msg.c_str() );
     
    int dest = static_cast<int>( ptr[0] );
    assert( dest >= 0 && dest < channels.size() );
    
   channels[dest]->write( msg );
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
    
}

void InputChannel::sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    unsigned char msg[4];
    msg[0] = static_cast<unsigned char>( _channelId );
    msg[1] = static_cast<unsigned char>( 0 /* send call event id */ );
    msg[2] = static_cast<unsigned char>( serviceId );
    msg[3] = static_cast<unsigned char>( methodIndex );
    
    std::string s;
    s.assign( reinterpret_cast<char*>( msg ), 4 );
    write( s );
}

void InputChannel::call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    unsigned char msg[4];
    msg[0] = static_cast<unsigned char>( _channelId );
    msg[1] = static_cast<unsigned char>( 1 /* send call event id */ );
    msg[2] = static_cast<unsigned char>( serviceId );
    msg[3] = static_cast<unsigned char>( methodIndex );
    
    std::string s;
    s.assign( reinterpret_cast<char*>( msg ), 4 );
    write( s );
}

void InputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
    write( "3" );
}

void InputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
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

void OutputChannel::sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    _delegate->onSendCall( this, serviceId, methodIndex, args );
}
    
void OutputChannel::call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result )
{
    _delegate->onCall( this, serviceId, methodIndex, args, result );
}
    
void OutputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    _delegate->onGetField( this, serviceId, fieldIndex, result );
}
    
void OutputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    _delegate->onSetField( this, serviceId, fieldIndex, value );
}
    
void OutputChannel::write( const std::string& rawMessage )
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>( rawMessage.c_str() );
    
    int eventType = static_cast<int>( ptr[1] );
    
    co::Range<co::Any const> r();   
    if( eventType == 0  )
    {
        newInstance( "toto.Toto" );
    
    }
    else if( eventType == 1 )
    {
        // first argument to CM is serviceId, second is methodId
        int serviceId = static_cast<int>( ptr[2] );
        int methodId =  static_cast<int>( ptr[3] );
        
        assert( serviceId >= 0 && methodId >= 0 );
        sendCall( serviceId, methodId, co::Range<co::Any const>() );
    }
}

} // namespace reef