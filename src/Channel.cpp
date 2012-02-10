#include "Channel.h"
#include "Message.pb.h"

#include <co/IMethod.h>
#include <co/IParameter.h>

#include <iostream>

#include "ServerNode.h"

namespace reef 
{
    
static void printChannelMessage( Message* message )
{
    std::cout << "MESSAGE INFO:" << std::endl;
    std::cout << "Destination:" << message->destination() << std::endl;
    
    std::cout << message->type() << std::endl;
}
    
static void convertCoType( const co::Any& any, DataContainer* container )
{
	co::TypeKind kind = any.getKind();

	switch( kind )
	{
	case co::TK_BOOLEAN:
		container->set_boolean( any.get<bool>() );
		break;
	case co::TK_INT8:
	case co::TK_UINT8:
	case co::TK_INT16:
	case co::TK_UINT16:
	case co::TK_INT32:
	case co::TK_UINT32:
	case co::TK_INT64:
	case co::TK_UINT64:
	case co::TK_FLOAT:
	case co::TK_DOUBLE:
		container->set_numeric( any.get<double>() );
		break;
	case co::TK_STRING:
		{
			const std::string& s = any.get<const std::string&>();
			container->set_str( s );
			break;
		}
	default:
		assert( false );
	}
}

static void convertDataType( const DataContainer& container, const co::TypeKind kind, co::Any& result )
{
	switch( kind )
	{
	case co::TK_BOOLEAN:
		result.set<bool>( container.boolean() );
	case co::TK_INT8:
		result.set<co::int8>( static_cast<co::int8>( container.numeric() ) );
	case co::TK_UINT8:
		result.set<co::uint8>( static_cast<co::uint8>( container.numeric() ) );
	case co::TK_INT16:
		result.set<co::int16>( static_cast<co::int16>( container.numeric() ) );
	case co::TK_UINT16:
		result.set<co::uint16>( static_cast<co::uint16>( container.numeric() ) );
	case co::TK_INT32:
		result.set<co::int32>( static_cast<co::int32>( container.numeric() ) );
	case co::TK_UINT32:
		result.set<co::uint32>( static_cast<co::uint32>( container.numeric() ) );
	case co::TK_INT64:
		result.set<co::int64>( static_cast<co::int64>( container.numeric() ) );
	case co::TK_UINT64:
		result.set<co::uint64>( static_cast<co::uint64>( container.numeric() ) );
	case co::TK_FLOAT:
		result.set<float>( static_cast<float>( container.numeric() ) );
	case co::TK_DOUBLE:
		result.set<double>( container.numeric() );
	case co::TK_STRING:
		result.set<const std::string&>( container.str() );
	default:
		assert( false );
	}
}

static void anyToPBArg( const co::Any& any, Argument* arg )
{
	DataContainer* container = arg->add_data();
	convertCoType( any, container );
}

static void PBArgToAny( const Argument& arg, co::IParameter* descriptor, co::Any& any )
{
	co::TypeKind kind = descriptor->getType()->getKind();
	convertDataType( arg.data( 0 ), kind, any ); 
}

static Message_Call* makeCallMessage( int destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_CALL );
	
    Message_Call* mc = owner.mutable_msgcall();
    mc->set_hasreturn( hasReturn );
    mc->set_serviceindex( serviceId );
    mc->set_memberindex( methodIndex );
    for( ; args; args.popFirst() )
	{
		Argument* arg = mc->add_arguments();
		anyToPBArg( args.getFirst(), arg );

	}
    
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
    _connecter->receiveReply( input );

    // TODO: properly set returned value into result variable and set all out values from args list
}

void InputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    Message message;
    makeFieldMessage( _channelId, false, message, serviceId, fieldIndex );

    write( &message );
    
    // wait for the field value
    std::string input;
    _connecter->receiveReply( input );
    
  /*  DataType fieldValue;
    fieldValue.ParseFromString( input );
    */
    // TODO: properly set the returned field value into result variable
   // result.set( fieldValue.dummy() );
}

void InputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    Message message;
    Message_Field* mf = makeFieldMessage( _channelId, true, message, serviceId, fieldIndex );

	Argument* arg = mf->mutable_value();
	anyToPBArg( value, arg );
    
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
        case Message::TYPE_CALL:
        {
            const Message_Call& callMsg = message->msgcall();
            co::int32 serviceId = callMsg.serviceindex();
            co::int32 memberIndex = callMsg.memberindex();
            
            // TODO: handle call arguments (translate to co::Any)
            // const DataArgument& argument = call.arguments( 0 );
            
            if( callMsg.hasreturn() )
            {
                //co::Any result;
                //call( serviceId, methodIndex, dummy, result );
                
                // TODO: handle return
            }
            else
            {
				co::IObject* instance = _owner->mapInstance( getId() );
				co::IMethod* method = co::cast<co::IMethod>( instance->getInterface()->getMembers()[memberIndex] );
				std::vector<co::Any> anyArgs;
				google::protobuf::RepeatedPtrField<Argument> pbArgs = callMsg.arguments();
				google::protobuf::RepeatedPtrField<Argument>::const_iterator it = pbArgs.begin();
				for( co::Range<co::IParameter* const> params = method->getParameters(); params; params.popFirst() )
				{
					co::Any anyArg;
					PBArgToAny( *it, params.getFirst(), anyArg );
					it++;
					anyArgs.push_back( anyArg );
				}
				
				sendCall( serviceId, memberIndex, anyArgs );
            }
            break;
        }
        default:
            break;
    }
}

} // namespace reef