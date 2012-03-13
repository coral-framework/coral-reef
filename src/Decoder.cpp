#include "Decoder.h"

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
    
// Decoder
Decoder::Decoder( Binder* binder )
    : _binder( binder ) 
{
}

Decoder::~Decoder()
{
    // empty
}

void Decoder::routeAndDeliver( const std::string& data, const std::vector<Channel*>& channels )
{
    Message message;
    message.ParseFromString( data );
    
    int dest = message.destination();
    assert( dest >= 0 && dest < channels.size() );
    deliver( &message, channels[dest] );
}
    
void Decoder::deliver( const std::string& data, Channel* destination )
{   
    Message message;
    message.ParseFromString( data );
    
    deliver( &message, destination );
}
    
void Decoder::deliver( Message* msg, Channel* destination )
{
    Message::Type type = msg->type();
    _destination = destination;
    
    switch ( type ) 
    {
        case Message::TYPE_NEW:
        {
            const Message_New& subMsg = msg->msgnew(); 
            deliverNew( &subMsg );
            break;
        }
        case Message::TYPE_FIELD:
        {
            const Message_Member& subMsg = msg->msgmember(); 
            deliverField( &subMsg );
            break;
        }
        case Message::TYPE_CALL:
        {
            const Message_Member& subMsg = msg->msgmember(); 
            deliverCall( &subMsg );
            break;
        }
        default:
            throw co::Exception( "Unsupported message type received" );
    }
}
int Decoder::newInstance( const std::string& typeName )
{
    return _destination->newInstance( typeName );
}

void Decoder::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    _destination->sendCall( serviceId, method, args );
}
    
void Decoder::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    _destination->call( serviceId, method, args, result );
}
    
void Decoder::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    _destination->getField( serviceId, field, result );
}
    
void Decoder::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    _destination->setField( serviceId, field, value );
}
      
void Decoder::deliverNew( const Message_New* subMessage )
{
    int virtualAddress = newInstance( subMessage->componenttypename() );

    VirtualAddress va;
    va.set_address( virtualAddress );
    
    std::string output;
    va.SerializeToString( &output );
    _binder->reply( output );
    
}        
void Decoder::deliverField( const Message_Member* subMessage )
{
    co::int32 serviceId = subMessage->serviceindex();
    co::int32 memberIndex = subMessage->memberindex();
    
    // TODO: handle call arguments (translate to co::Any)
    // const DataArgument& argument = call.arguments( 0 );
    
    co::IPort* port = _destination->getComponent()->getPorts()[serviceId];
    co::IInterface* iface = port->getType();
    
    co::IMember* member = iface->getMembers()[memberIndex];
    co::IField* field =  co::cast<co::IField>( member );
    
    if( subMessage->hasreturn() ) // getField
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
        const Argument& pbArg = subMessage->arguments( 0 );
        MessageUtils::PBArgToAny( pbArg, field->getType(), value );
        setField( serviceId, field, value );
    }
}
void Decoder::deliverCall( const Message_Member* subMessage )
{
    co::int32 serviceId = subMessage->serviceindex();
    co::int32 memberIndex = subMessage->memberindex();
    
    // TODO: handle call arguments (translate to co::Any)
    // const DataArgument& argument = call.arguments( 0 );
    
    co::IPort* port = _destination->getComponent()->getPorts()[serviceId];
    co::IInterface* iface = port->getType();

    co::IMember* member = iface->getMembers()[memberIndex];
    co::IMethod* method =  co::cast<co::IMethod>( member );
    co::Range<co::IParameter* const> params = method->getParameters();

    std::vector<co::Any> anyArgs;
    google::protobuf::RepeatedPtrField<Argument> pbArgs = subMessage->arguments();
    google::protobuf::RepeatedPtrField<Argument>::const_iterator it = pbArgs.begin();
    size_t size = pbArgs.size();
    anyArgs.resize( size );
    for( int i = 0; i < size; i++ )
    {
        MessageUtils::PBArgToAny( *it, params.getFirst()->getType(), anyArgs[i] );
        it++;
        params.popFirst();
    }
    
    if( !subMessage->hasreturn() )
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
}

} // namespace reef