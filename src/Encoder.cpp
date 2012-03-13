#include "Encoder.h"

#include "Message.pb.h"
#include "MessageUtils.h"
#include "network/Connection.h"

#include <co/IField.h>
#include <co/IMethod.h>

namespace reef {
    
// Encoder
Encoder::Encoder( Connecter* connecter ) : _connecter( connecter )
{   
}

Encoder::~Encoder()
{
    // empty
}            

// Blocking function
void Encoder::fetchReturnValue( co::IType* descriptor, co::Any& returnValue )
{
	std::string input;
    _connecter->receiveReply( input );
	Argument arg;
	arg.ParseFromString( input );
	MessageUtils::PBArgToAny( arg, descriptor, returnValue );
}

int Encoder::newInstance( const std::string& typeName )
{
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
    
    _decoderAddress = va.address();
    
    return _decoderAddress;
}

void Encoder::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // make a call without returns
    Message message;
    MessageUtils::makeCallMessage( _decoderAddress, false, message, serviceId, method->getIndex(), args );
    write( &message );
}

void Encoder::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // make a call with returns
    Message message;
    MessageUtils::makeCallMessage( _decoderAddress, true, message, serviceId, method->getIndex(), args );
    
    write( &message );
    
	fetchReturnValue( method->getReturnType(), result );
    
    // TODO: properly set returned value into result variable and set all out values from args list
}

void Encoder::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    Message message;
    MessageUtils::makeGetFieldMessage( _decoderAddress, message, serviceId, field->getIndex() );
    
    write( &message );
    
    fetchReturnValue( field->getType(), result );
}

void Encoder::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    Message message;
    MessageUtils::makeSetFieldMessage( _decoderAddress, message, serviceId, field->getIndex(), value );
    
    write( &message );
}

void Encoder::write( const Message* message )
{
    std::string output;
    message->SerializeToString( &output );
    
    _connecter->send( output );
}

}
