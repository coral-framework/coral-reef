#include "Channel.h"
#include "Message.pb.h"

#include <iostream>

namespace reef 
{
    
static void printChannelMessage( Message* message )
{
    std::cout << "MESSAGE INFO:" << std::endl;
    std::cout << "Destination:" << message->destination() << std::endl;
    
    std::cout << message->type() << std::endl;
}
    
    
static Message_Call* makeCallMessage( int destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_CALL );
    
    Message_Call* mc = owner.mutable_msgcall();
    mc->set_hasreturn( hasReturn );
    mc->set_serviceindex( serviceId );
    mc->set_methodindex( methodIndex );
    
    // TODO: serialize parameters
    return mc;
}
    
static Message_Field* makeFieldMessage( int destination, bool isSet, Message& owner, co::int32 serviceId, co::int32 fieldIndex )
{
    owner.set_destination( destination ); // 0 is always the node channel
    owner.set_type( Message::TYPE_FIELD );
    
    Message_Field* mf = owner.mutable_msgfield();
    mf->set_issetfield( isSet ); // it is a get field event
    mf->set_serviceindex( serviceId );
    mf->set_fieldindex( fieldIndex );
    
    return mf;
}
    
void Channel::route( const std::string& data, const std::vector<Channel*>& channels )
{
    Message message;
    message.ParseFromString( data );
    
    printChannelMessage( &message );

    int dest = message.destination();
    assert( dest >= 0 && dest < channels.size() );
    
    channels[dest]->write( &message );
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
    
    Message message;
    message.set_destination( 0 ); // 0 is always the node channel
    message.set_type( Message::TYPE_NEW );
    
	Message_New* msgNew = message.mutable_msgnew();
    msgNew->set_componenttypename( typeName );
    
    write( &message );
    
    std::string input;
    _connection->receive( input );
    
    VirtualAddress va;
    va.ParseFromString( input );
    
    int virtualAddress = va.address();
    assert( virtualAddress > 0 );
 
    _channelId = va.address();
    
    return _channelId;
}
                                
void InputChannel::sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    // make a call without returns
    Message message;
    makeCallMessage( _channelId, false, message, serviceId, methodIndex, args );
    write( &message );
}

void InputChannel::call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result )
{
    // make a call with returns
    Message message;
    makeCallMessage( _channelId, true, message, serviceId, methodIndex, args );
    
    write( &message );
    
    // wait for the return
    std::string input;
    _connection->receive( input );
    
    ReturnData ret;
    ret.ParseFromString( input );
    
    // TODO: properly set returned value into result variable and set all out values from args list
    result.set( static_cast<co::int32>( ret.arguments( 0 ).dummy() ) );
}

void InputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    Message message;
    makeFieldMessage( _channelId, false, message, serviceId, fieldIndex );

    write( &message );
    
    // wait for the field value
    std::string input;
    _connection->receive( input );
    
    DataType fieldValue;
    fieldValue.ParseFromString( input );
    
    // TODO: properly set the returned field value into result variable
    result.set( fieldValue.dummy() );
}

void InputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    Message message;
    Message_Field* mf = makeFieldMessage( _channelId, true, message, serviceId, fieldIndex );
    // TODO: properly set field value
    mf->mutable_value()->set_dummy( value.get<int>() );
    
    write( &message );
}
    
void InputChannel::write( const Message* message )
{
    std::string output;
    message->SerializeToString( &output );
    
    _connection->send( output );
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
    
void OutputChannel::write( const Message* message )
{    
    co::Range<co::Any const> dummy;

    Message::Type type = message->type();
    
    switch ( type ) {
        case Message::TYPE_NEW:
        {
            const Message_New& msgNew = message->msgnew();
            int virtualAddress = newInstance( msgNew.componenttypename() );

            VirtualAddress va;
            va.set_address( virtualAddress );
            
            std::string output;
            va.SerializeToString( &output );
            _connection->send( output );
            
            break;
        } 
        case Message::TYPE_CALL:
        {
            const Message_Call& callMsg = message->msgcall();
            co::int32 serviceId = callMsg.serviceindex();
            co::int32 methodIndex = callMsg.methodindex();
            
            // TODO: handle call arguments (translate to co::Any)
            // const DataArgument& argument = call.arguments( 0 );
            
            if( callMsg.hasreturn() )
            {
                co::Any result;
                call( serviceId, methodIndex, dummy, result );
                
                // TODO: handle return
            }
            else
            {
                sendCall( serviceId, methodIndex, dummy );
            }
            break;
        }
        default:
            break;
    }
}

} // namespace reef