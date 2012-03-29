#include "MessageBuilder.h"

#include "Message.pb.h"
#include "MessageUtils.h"
#include "network/Connection.h"

#include <co/IField.h>
#include <co/IMethod.h>

namespace reef
{
    
MessageEncoder::MessageEncoder()
{
    _message = new Message();
}

MessageEncoder::~MessageEncoder()
{
    delete _message;
}
    
void MessageEncoder::newNewInstanceMsg( const std::string& typeName, std::string& msg )
{
    _message->set_destination( 0 ); // 0 is always the node channel
    _message->set_type( Message::TYPE_NEW );
    
	Message_New* msgNew = _message->mutable_msgnew();
    msgNew->set_componenttypename( typeName );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void MessageEncoder::buildCallMsg( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx )
{
    _message->set_destination( instanceID );
    _message->set_type( Message::TYPE_CALL );
    
    _msgMember = _message->mutable_msgmember();
    _msgMember->set_serviceindex( facetIdx );
    _msgMember->set_memberindex( memberIdx );
}

void MessageEncoder::addValueParam( const co::Any param )
{
    checkIfCallMsg();
    Argument* PBArg = _msgMember->add_arguments();
    MessageUtils::anyToPBArg( param, PBArg );
}

void MessageEncoder::addLocalRefParam( co::int32 instanceID, co::int32 facetIdx )
{
    RefType* refType = makeRefParam( instanceID, facetIdx );
    refType->set_owner( RefType::SELF );
}

void MessageEncoder::addReceiverRefParam( co::int32 instanceID, co::int32 facetIdx )
{
    RefType* refType = makeRefParam( instanceID, facetIdx );
    refType->set_owner( RefType::RECEIVER );
}

void MessageEncoder::addAnotherRefParam( co::int32 instanceID, co::int32 facetIdx, 
                                            const std::string& ownerAddress )
{
    RefType* refType = makeRefParam( instanceID, facetIdx );
    refType->set_owner( RefType::THIRD_PARTY );
    refType->set_ownerip( ownerAddress );
}

void MessageEncoder::getBuiltCallMsg( std::string& msg )
{
    _message->SerializeToString( &msg );
    _message->Clear();
    _msgMember = 0;
}

void MessageEncoder::newDataMsg( bool value, std::string& msg )
{
    DataContainer data;
    data.set_boolean( value );
    data.SerializeToString( &msg );
}

void MessageEncoder::newDataMsg( double value, std::string& msg )
{
    DataContainer data;
    data.set_numeric( value );
    data.SerializeToString( &msg );
}

void MessageEncoder::newDataMsg( co::int32 value, std::string& msg )
{
    DataContainer data;
    data.set_numeric( static_cast<co::int32>( value ) );
    data.SerializeToString( &msg );
}

void MessageEncoder::newDataMsg( const std::string& value, std::string& msg )
{
    DataContainer data;
    data.set_str( value );
    data.SerializeToString( &msg );
}

RefType* MessageEncoder::makeRefParam( co::int32 instanceID, co::int32 facetIdx )
{
    checkIfCallMsg();
    Argument* PBArg = _msgMember->add_arguments();
    DataContainer* dc = PBArg->add_data();
    RefType* refType = dc->mutable_reftype();
    
    refType->set_owner( RefType::SELF );
    refType->set_virtualaddress( instanceID );
    refType->set_interfaceindex( facetIdx );
    
    return refType;
}
    
void MessageEncoder::checkIfCallMsg()
{
    if( !_msgMember )
        throw new co::Exception( "Could not add a Parameter to an empty Message" );
}
    
}