#include "Channel.h"
#include "Message.pb.h"

#include <co/IMethod.h>
#include <co/IField.h>
#include <co/IParameter.h>

#include <iostream>

#include "ServerNode.h"

#include "MessageUtils.h"

namespace reef 
{

static void printChannelMessage( Message* message )
{
    std::cout << "MESSAGE INFO:" << std::endl;
    std::cout << "Destination:" << message->destination() << std::endl;
    
    std::cout << message->type() << std::endl;
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
    
Channel::Channel() : _channelId( -1 )
{
    // empty
}

Channel::~Channel()
{
    // empty
}

// InputChannel
InputChannel::InputChannel( Connecter* connecter ) : _connecter( connecter )
{   
}
    
InputChannel::~InputChannel()
{
    // empty
}            

// Blocking function
void InputChannel::fetchReturnValue( co::IType* descriptor, co::Any& returnValue )
{
	std::string input;
    _connecter->receiveReply( input );
	Argument arg;
	arg.ParseFromString( input );
	MessageUtils::PBArgToAny( arg, descriptor, returnValue );
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
    _connecter->receiveReply( input );
    
    VirtualAddress va;
    va.ParseFromString( input );
    
    int virtualAddress = va.address();
    assert( virtualAddress > 0 );
 
    _channelId = va.address();
    
    return _channelId;
}
                                
void InputChannel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // make a call without returns
    Message message;
    MessageUtils::makeCallMessage( _channelId, false, message, serviceId, method->getIndex(), args );
    write( &message );
}

void InputChannel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // make a call with returns
    Message message;
    MessageUtils::makeCallMessage( _channelId, true, message, serviceId, method->getIndex(), args );
    
    write( &message );
    
	fetchReturnValue( method->getReturnType(), result );

    // TODO: properly set returned value into result variable and set all out values from args list
}

void InputChannel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    Message message;
    MessageUtils::makeGetFieldMessage( _channelId, message, serviceId, field->getIndex() );

    write( &message );
    
    fetchReturnValue( field->getType(), result );
}

void InputChannel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    Message message;
    MessageUtils::makeSetFieldMessage( _channelId, message, serviceId, field->getIndex(), value );
    
    write( &message );
}
    
void InputChannel::write( const Message* message )
{
    std::string output;
    message->SerializeToString( &output );
    
    _connecter->send( output );
}

// OutputChannel
OutputChannel::OutputChannel( ServerNode* owner, Binder* binder )
    : _binder( binder ) 
{
	_owner = owner;
	assert( owner );
}

OutputChannel::~OutputChannel()
{
    // empty
}

void OutputChannel::setDelegate( OutputChannelDelegate* delegate )
{
	_delegate = delegate;
}

int OutputChannel::newInstance( const std::string& typeName )
{
    return _delegate->onNewInstance( this, typeName );
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
    
void OutputChannel::write( const Message* message )
{    
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
            _binder->reply( output );
            
            break;
        }
        case Message::TYPE_FIELD:
        {
            const Message_Member& callMsg = message->msgmember();
            co::int32 serviceId = callMsg.serviceindex();
            co::int32 memberIndex = callMsg.memberindex();
            
            // TODO: handle call arguments (translate to co::Any)
            // const DataArgument& argument = call.arguments( 0 );
            
			co::IObject* instance = _owner->mapInstance( getId() );
			co::IPort* port = instance->getComponent()->getPorts()[serviceId];
			co::IInterface* iface = port->getType();
            
			co::IMember* member = iface->getMembers()[memberIndex];
			co::IField* field =  co::cast<co::IField>( member );
            
            if( callMsg.hasreturn() ) // getField
            {
                co::Any returnValue;
				getField( serviceId, field, returnValue );
                
				Argument returnArg;
				MessageUtils::anyToPBArg( returnValue, &returnArg );
				std::string output;
				returnArg.SerializeToString( &output );
    			_binder->reply( output );
            }
            else // setField
            {
                co::Any value;
                const Argument& pbArg = callMsg.arguments( 0 );
                MessageUtils::PBArgToAny( pbArg, field->getType(), value );
                setField( serviceId, field, value );
            }
            
            break;
        }
        case Message::TYPE_CALL:
        {
            const Message_Member& callMsg = message->msgmember();
            co::int32 serviceId = callMsg.serviceindex();
            co::int32 memberIndex = callMsg.memberindex();
            
            // TODO: handle call arguments (translate to co::Any)
            // const DataArgument& argument = call.arguments( 0 );
            
			co::IObject* instance = _owner->mapInstance( getId() );
			co::IPort* port = instance->getComponent()->getPorts()[serviceId];
			co::IInterface* iface = port->getType();

			co::IMember* member = iface->getMembers()[memberIndex];
			co::IMethod* method =  co::cast<co::IMethod>( member );
			co::Range<co::IParameter* const> params = method->getParameters();

			std::vector<co::Any> anyArgs;
			google::protobuf::RepeatedPtrField<Argument> pbArgs = callMsg.arguments();
			google::protobuf::RepeatedPtrField<Argument>::const_iterator it = pbArgs.begin();
			size_t size = pbArgs.size();
			anyArgs.resize( size );
			for( int i = 0; i < size; i++ )
			{
				MessageUtils::PBArgToAny( *it, params.getFirst()->getType(), anyArgs[i] );
				it++;
				params.popFirst();
			}
			
			if( !callMsg.hasreturn() )
			{
				sendCall( serviceId, method, anyArgs );
			}
			else
			{
				co::Any returnValue;
				call( serviceId, method, anyArgs, returnValue );

				Argument returnArg;
				MessageUtils::anyToPBArg( returnValue, &returnArg );
				std::string output;
				returnArg.SerializeToString( &output );
    			_binder->reply( output );
			}

            break;
        }
        default:
            break;
    }
}

} // namespace reef