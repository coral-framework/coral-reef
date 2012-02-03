#include "Channel.h"

namespace reef 
{
    
void Channel::route( const void* data, unsigned int size, const std::vector<Channel*>& channels )
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>( data );
     
    int dest = static_cast<int>( ptr[0] );
    assert( dest >= 0 && dest < channels.size() );
    
   channels[dest]->write( data, size );
}
    
Channel::Channel( Connection* connection ) : _channelId( -1 ), _connection( connection )
{
    // empty
}

Channel::~Channel()
{
    // empty
}

// InputChannel
    InputChannel::InputChannel( Connection* connection ) : Channel( connection )
{   
    _connection = connection;
}
    
InputChannel::~InputChannel()
{
    // empty
}            

int InputChannel::newInstance( const std::string& typeName )
{
    assert( _channelId == - 1 );
    
    // translate arguments (google protocol buffers?)
    // send to current connection
    unsigned char msg[4];
    msg[0] = static_cast<unsigned char>( 0 /* server id */ );
    msg[1] = static_cast<unsigned char>( 0 );
    msg[2] = static_cast<unsigned char>( 0 );
    msg[3] = static_cast<unsigned char>( 0 );
    
    write( msg, 4 );
    
    _connection->receive( reinterpret_cast<unsigned char*>( msg ), 4 );
    unsigned char id = msg[0];
    _channelId = static_cast<int>( id );
    
    return _channelId;
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
    
    write( msg, 4 );

    _connection->receive( reinterpret_cast<unsigned char*>( msg ), 4 );
    unsigned char id = msg[0];
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
    
    write( msg, 4 );
    
    _connection->receive( reinterpret_cast<unsigned char*>( msg ), 4 );
    unsigned char id = msg[0];
}

void InputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}

void InputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    // translate arguments (google protocol buffers?)
    // send to current connection
}
    
void InputChannel::write( const void* rawMessage, unsigned int size )
{
    _connection->send( rawMessage, size );
}
    
// OutputChannel
OutputChannel::OutputChannel( Connection* connection, OutputChannelDelegate* delegate )
    : Channel( connection ) 
{
    _delegate = delegate;
}
    
OutputChannel::~OutputChannel()
{
    // empty
}

int OutputChannel::newInstance( const std::string& typeName )
{
    return _delegate->onNewInstance( this, typeName );
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
    
void OutputChannel::write( const void* data, unsigned int size )
{
    const unsigned char* ptr = reinterpret_cast<const unsigned char*>( data );
    
    int eventType = static_cast<int>( ptr[1] );
    
    co::Range<co::Any const> r();   
    if( eventType == 0  )
    {
        int ret = newInstance( "toto.Toto" );
        
        unsigned char msg[4];
        msg[0] = static_cast<unsigned char>( ret );
        msg[1] = static_cast<unsigned char>( 0  );
        msg[2] = static_cast<unsigned char>( 0 );
        msg[3] = static_cast<unsigned char>( 0 );
        _connection->send( reinterpret_cast<char*>( msg ), 4 );
    
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