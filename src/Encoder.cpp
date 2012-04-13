#include "Encoder.h"
#include "RemoteObject.h"

#include "Message.pb.h"
#include "MessageUtils.h"
#include "network/Transport.h"

#include <co/IField.h>
#include <co/IMethod.h>

namespace reef {
    
// Encoder
Encoder::Encoder( Connecter* connecter, IServerNode* publisher ) : 
    _connecter( connecter ), _publisher( publisher )
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
    
    DataContainer instanceAddress;
    instanceAddress.ParseFromString( input );
    
    _instanceAddress = instanceAddress.numeric();
    
    return _instanceAddress;
}

void Encoder::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
    // make a call without returns
    Message message;
    makeCallMessage( _instanceAddress, false, message, serviceId, method->getIndex(), args );
    write( &message );
}

void Encoder::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    // make a call with returns
    Message message;
    makeCallMessage( _instanceAddress, true, message, serviceId, method->getIndex(), args );
    
    write( &message );
    
	fetchReturnValue( method->getReturnType(), result );
    
    // TODO: properly set returned value into result variable and set all out values from args list
}

void Encoder::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    Message message;
    makeGetFieldMessage( _instanceAddress, message, serviceId, field->getIndex() );
    
    write( &message );
    
    fetchReturnValue( field->getType(), result );
}

void Encoder::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    Message message;
    makeSetFieldMessage( _instanceAddress, message, serviceId, field->getIndex(), value );
    
    write( &message );
}

void Encoder::write( const Message* message )
{
    std::string output;
    message->SerializeToString( &output );
    
    _connecter->send( output );
}

    
void Encoder::makeCallMessage( co::int32 destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_CALL );
    
    Message_Member* mc = owner.mutable_msgmember();
    mc->set_hasreturn( hasReturn );
    mc->set_serviceindex( serviceId );
    mc->set_memberindex( methodIndex );
    for( ; args; args.popFirst() )
    {
        Argument* PBArg = mc->add_arguments();
        MessageUtils::anyToPBArg( args.getFirst(), PBArg );
    }
}

void Encoder::makeSetFieldMessage( co::int32 destination, Message& owner, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_FIELD );
    
    Message_Member* mf = owner.mutable_msgmember();
    mf->set_serviceindex( serviceId );
    mf->set_hasreturn( false ); // it is a set field event
    mf->set_memberindex( fieldIndex );
    
    Argument* PBArg = mf->add_arguments();
    MessageUtils::anyToPBArg( value, PBArg );
}

void Encoder::makeGetFieldMessage( co::int32 destination, Message& owner, co::int32 serviceId, co::int32 fieldIndex )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_FIELD );
    
    Message_Member* mf = owner.mutable_msgmember();
    mf->set_serviceindex( serviceId );
    mf->set_hasreturn( true ); // it is a get field event
    mf->set_memberindex( fieldIndex );
}

void Encoder::convertArgs( Message_Member* msgMember, co::Range<co::Any const> args )
{
    for( ; args; args.popFirst() )
    {
        Argument* PBArg = msgMember->add_arguments();
        
        if( args.getFirst().getKind() != co::TK_INTERFACE )
            MessageUtils::anyToPBArg( args.getFirst(), PBArg );
        else
            convertRefTypeArg( args.getFirst(), PBArg );
    }
}
    
void Encoder::convertRefTypeArg( const co::Any refType, Argument* PBArg )
{
    co::int32 virtualAddress;
    co::int32 interfaceIndex;
    co::int32 addressOwnerType;
    std::string AddressOwnerIP;
    
    co::IService* service = refType.get<co::IService*>();
    interfaceIndex = service->getFacet()->getIndex();
    co::IObject* provider = service->getProvider();
    
    if( RemoteObject::isLocalObject( provider ) )
    {
        addressOwnerType = 0;
        virtualAddress = _publisher->publishInstance( provider );
    }
    else
    {
        
    }
}

}
