#include "Unmarshaller.h"

#include "Message.pb.h"

#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>

namespace reef {
namespace rpc {

    // -------------- Protobuf to Any conversion functions ------------------//
    
// Specializes for each Data container's different get function.
template <typename T>
static T getPBContainerData( const Data_Container& container )
{
    return static_cast<T>( container.numeric() );
}

// ------------- get and set functions specialization for string and bool ----------- //
template <>
const std::string& getPBContainerData<const std::string&>( const Data_Container& container )
{
    return container.str();
}

template <>
bool getPBContainerData<bool>( const Data_Container& container )
{
    return container.boolean();
}
    
// Extracts the provided type's data from Argument (deals with arrays and values)
template <typename T>
static void PBArgWithTypeToAny( const Argument& arg, co::Any& any, co::IType* elementType )
{
    if( !elementType )
    {
        any.set<T>( getPBContainerData<T>( arg.data( 0 ) ) );
        return;
    }
    
    size_t size = arg.data().size();
    if( size == 0 ) // required for vector subscript out of range assertion
        return;
    
    std::vector<co::uint8>& vec = any.createArray( elementType, size );
    T* toCast = reinterpret_cast<T*>( &vec[0] );
    for( int i = 0; i < size; i++ )
    {
        toCast[i] = getPBContainerData<T>( arg.data( i ) );
    }
}

// ----------------- PBArgWithTypeToAny specializations for string and bool --------------- //
template <>
void PBArgWithTypeToAny<std::string>( const Argument& arg, co::Any& any, co::IType* elementType )
{
    if( !elementType )
    {
        std::string& anyString = any.createString();
        anyString = getPBContainerData<const std::string&>( arg.data( 0 ) );
        return;
    }
    
    size_t size = arg.data().size();
    if( size == 0 ) // required for vector subscript out of range assertion
        return;
    
    std::vector<co::uint8>& vec = any.createArray( elementType, size );
    std::string* toCast = reinterpret_cast<std::string*>( &vec[0] );
    for( int i = 0; i < size; i++ )
    {
        toCast[i] = getPBContainerData<const std::string&>( arg.data( i ) );
    }
}

void PBArgToAny( const Argument& arg, co::IType* descriptor, co::Any& any )
{
    co::TypeKind kind = descriptor->getKind();
    co::IType* elementType = 0; // only used for arrays
    
    if( kind == co::TK_ARRAY )
    {
        elementType = co::cast<co::IArray>( descriptor )->getElementType();
        kind = elementType->getKind();
    }
    
    switch( kind )
    {
        case co::TK_BOOLEAN:
            PBArgWithTypeToAny<bool>( arg, any, elementType );
            break;
        case co::TK_INT8:
            PBArgWithTypeToAny<co::int8>( arg, any, elementType );
            break;
        case co::TK_UINT8:
            PBArgWithTypeToAny<co::uint8>( arg, any, elementType );
            break;
        case co::TK_INT16:
            PBArgWithTypeToAny<co::int16>( arg, any, elementType );
            break;
        case co::TK_UINT16:
            PBArgWithTypeToAny<co::uint16>( arg, any, elementType );
            break;
        case co::TK_INT32:
            PBArgWithTypeToAny<co::int32>( arg, any, elementType );
            break;
        case co::TK_UINT32:
            PBArgWithTypeToAny<co::uint32>( arg, any, elementType );
            break;
        case co::TK_INT64:
            PBArgWithTypeToAny<co::int64>( arg, any, elementType );
            break;
        case co::TK_UINT64:
            PBArgWithTypeToAny<co::uint64>( arg, any, elementType );
            break;
        case co::TK_FLOAT:
            PBArgWithTypeToAny<float>( arg, any, elementType );
            break;
        case co::TK_DOUBLE:
            PBArgWithTypeToAny<double>( arg, any, elementType );
            break;
        case co::TK_STRING:
            PBArgWithTypeToAny<std::string>( arg, any, elementType );
            break;
        default:
            assert( false );
    }
}
    
void Message_Type2MsgType( Message_Type message_type, Unmarshaller::MsgType& msgType )
{
    switch( message_type )
    {
        case Message::MSG_NEW_INST:
            msgType = Unmarshaller::NEW_INST;
            break;
        case Message::MSG_ACCESS_INST:
            msgType = Unmarshaller::ACCESS_INST;
            break;
        case Message::MSG_FIND_INST:
            msgType = Unmarshaller::FIND_INST;
            break;
        case Message::MSG_CALL:
            msgType = Unmarshaller::CALL;
            break;
    }
}
    

    
Unmarshaller::Unmarshaller() : _currentParam( 0 )
{
    _message = new Message();
    _argument = new Argument();
}

Unmarshaller::~Unmarshaller()
{
    delete _message;
    delete _argument;
}
    
// sets the message that will be decoded and return its type and destination
void Unmarshaller::setMarshalledRequest( const std::string& request, MsgType& type, co::int32& instanceID, 
                                bool& hasReturn, std::string* referer )
{
    _message->Clear();
    _message->ParseFromString( request );
    instanceID = _message->instance_id();
    hasReturn = _message->has_return();
    Message_Type2MsgType( _message->msg_type(), type );
    _msgType = type;
    
    if( type != CALL )
        *referer = _message->referer_ip();
}

// if msg type is NEW, then, this function will decode it
void Unmarshaller::unmarshalNewInstance( std::string& typeName )
{
    assert( _msgType == NEW_INST );
    
    const Message_New_Inst& msgNewInst = _message->msg_new_inst();
    typeName = msgNewInst.new_instance_type();
}

void Unmarshaller::unmarshalAccessInstance( co::int32& instanceID, bool& increment )
{
    assert( _msgType == ACCESS_INST );
    
    const Message_Acc_Inst& msgAccessInst = _message->msg_acc_inst();
    // refererIP = msgAccessInst.referer_ip();
    instanceID = msgAccessInst.instance_id();
    increment = msgAccessInst.increment();
}
   
void Unmarshaller::unmarshalFindInstance( std::string& key )
{
    assert( _msgType == FIND_INST );
    
    const Message_Find_Inst& msgFindInst = _message->msg_find_inst();
    key = msgFindInst.key();
}
/* 
 Starts a decoding state of call/field msg. 
 The decoding state will only be reset after all params are decoded.
 */
void Unmarshaller::beginUnmarshallingCall( co::int32& facetIdx, co::int32& memberIdx )
{
    assert( _msgType == CALL );
    
    _msgMember = &_message->msg_member();
    _currentParam = 0;
    facetIdx = _msgMember->facet_idx();
    memberIdx = _msgMember->member_idx();
}

void Unmarshaller::unmarshalValueParam( co::Any& param, co::IType* descriptor )
{
    assert( _msgMember );
    PBArgToAny( _msgMember->arguments( _currentParam++ ), descriptor, param );
}

void Unmarshaller::unmarshalReferenceParam( co::int32& instanceID, co::int32& facetIdx, RefOwner& owner,
                  std::string& instanceType, std::string& ownerAddress )
{
    assert( _msgMember );
    const Ref_Type& refType = _msgMember->arguments( _currentParam++ ).data( 0 ).ref_type();
    instanceID = refType.instance_id();
    facetIdx = refType.facet_idx();
    
    switch( refType.owner() )
    {
        case Ref_Type::OWNER_LOCAL:
            owner = LOCAL;
            ownerAddress = refType.owner_ip();
            instanceType = refType.instance_type();
            return;
        case Ref_Type::OWNER_RECEIVER:
            owner = RECEIVER;
            return;
        case Ref_Type::OWNER_ANOTHER:
            owner = ANOTHER;
            ownerAddress = refType.owner_ip();
            instanceType = refType.instance_type();
            return;
    }
}
    
void Unmarshaller::unmarshalReference( const std::string& data, co::int32& instanceID, 
                                           co::int32& facetIdx, RefOwner& owner, 
                                           std::string& instanceType, std::string& ownerAddress )
{
    _argument->Clear();
    _argument->ParseFromString( data );
    
    Ref_Type* refType = _argument->mutable_data( 0 )->mutable_ref_type();
    
    instanceID = refType->instance_id();
    facetIdx = refType->facet_idx();
    
    switch( refType->owner() )
    {
        case Ref_Type::OWNER_LOCAL:
            owner = RefOwner::LOCAL;
            ownerAddress = refType->owner_ip();
            instanceType = refType->instance_type();
            return;
        case Ref_Type::OWNER_RECEIVER:
            owner = RefOwner::RECEIVER;
            return;
        case Ref_Type::OWNER_ANOTHER:
            owner = RefOwner::ANOTHER;
            ownerAddress = refType->owner_ip();
            instanceType = refType->instance_type();
            return;
    }
}
    
void Unmarshaller::unmarshalValue( const std::string& data, co::IType* descriptor, co::Any& value )
{
    Argument arg;
    arg.ParseFromString( data );
    PBArgToAny( arg, descriptor, value );
}
    
// ----- Data Container codec ----- //
void Unmarshaller::unmarshalData( const std::string& data, bool& value )
{
    Data_Container dataContainer;
    dataContainer.ParseFromString( data );
    value = dataContainer.boolean();
}

void Unmarshaller::unmarshalData( const std::string& data, double& value )
{
    Data_Container dataContainer;
    dataContainer.ParseFromString( data );
    value = dataContainer.numeric();
}

void Unmarshaller::unmarshalData( const std::string& data, co::int32& value )
{
    Data_Container dataContainer;
    dataContainer.ParseFromString( data );
    value = static_cast<co::int32>( dataContainer.numeric() );
}

void Unmarshaller::unmarshalData( const std::string& data, std::string& value )
{
    Data_Container dataContainer;
    dataContainer.ParseFromString( data );
    value = dataContainer.str();
}

}
    
}