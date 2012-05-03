#include "Decoder.h"

#include "Message.pb.h"

#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>

namespace reef
{
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
    
void Message_Type2MsgType( Message_Type message_type, Decoder::MsgType& msgType )
{
    switch( message_type )
    {
        case Message::MSG_NEW_INST:
            msgType = Decoder::MsgType::NEW_INST;
            break;
        case Message::MSG_ACCESS_INST:
            msgType = Decoder::MsgType::ACCESS_INST;
            break;
        case Message::MSG_FIND_INST:
            msgType = Decoder::MsgType::FIND_INST;
            break;
        case Message::MSG_CALL:
            msgType = Decoder::MsgType::CALL;
            break;
    }
}
    

    
Decoder::Decoder() : _currentParam( 0 )
{
    _message = new Message();
}

Decoder::~Decoder()
{
    delete _message;
}
    
// sets the message that will be decoded and return its type and destination
void Decoder::setMsgForDecoding( const std::string& msg, MsgType& type, co::int32& instanceID, 
                                bool& hasReturn, std::string* referer )
{
    _message->Clear();
    _message->ParseFromString( msg );
    instanceID = _message->instance_id();
    hasReturn = _message->has_return();
    Message_Type2MsgType( _message->msg_type(), type );
    _msgType = type;
    
    if( type != MsgType::CALL )
        *referer = _message->referer_ip();
}

// if msg type is NEW, then, this function will decode it
void Decoder::decodeNewInstMsg( std::string& typeName )
{
    assert( _msgType == MsgType::NEW_INST );
    
    const Message_New_Inst& msgNewInst = _message->msg_new_inst();
    typeName = msgNewInst.new_instance_type();
}

void Decoder::decodeAccessInstMsg( co::int32& instanceID, bool& increment )
{
    assert( _msgType == MsgType::ACCESS_INST );
    
    const Message_Acc_Inst& msgAccessInst = _message->msg_acc_inst();
    // refererIP = msgAccessInst.referer_ip();
    instanceID = msgAccessInst.instance_id();
    increment = msgAccessInst.increment();
}
   
void Decoder::decodeFindInstMsg( std::string& key )
{
    assert( _msgType == MsgType::FIND_INST );
    
    const Message_Find_Inst& msgFindInst = _message->msg_find_inst();
    key = msgFindInst.key();
}
/* 
 Starts a decoding state of call/field msg. 
 The decoding state will only be reset after all params are decoded.
 */
void Decoder::beginDecodingCallMsg( co::int32& facetIdx, co::int32& memberIdx )
{
    assert( _msgType == MsgType::CALL );
    
    _msgMember = &_message->msg_member();
    _currentParam = 0;
    facetIdx = _msgMember->facet_idx();
    memberIdx = _msgMember->member_idx();
}

void Decoder::getValueParam( co::Any& param, co::IType* descriptor )
{
    assert( _msgMember );
    PBArgToAny( _msgMember->arguments( _currentParam++ ), descriptor, param );
}

void Decoder::getRefParam( co::int32& instanceID, co::int32& facetIdx, RefOwner& owner,
                  std::string& instanceType, std::string& ownerAddress )
{
    assert( _msgMember );
    const Ref_Type& refType = _msgMember->arguments( _currentParam++ ).data( 0 ).ref_type();
    instanceID = refType.instance_id();
    facetIdx = refType.facet_idx();
    
    switch( refType.owner() )
    {
        case Ref_Type::OWNER_LOCAL:
            owner = RefOwner::LOCAL;
            ownerAddress = refType.owner_ip();
            instanceType = refType.instance_type();
            return;
        case Ref_Type::OWNER_RECEIVER:
            owner = RefOwner::RECEIVER;
            return;
        case Ref_Type::OWNER_ANOTHER:
            owner = RefOwner::ANOTHER;
            ownerAddress = refType.owner_ip();
            instanceType = refType.instance_type();
            return;
    }
}

// ----- Data Container codec ----- //
void Decoder::decodeData( const std::string& msg, bool& value )
{
    Data_Container data;
    data.ParseFromString( msg );
    value = data.boolean();
}

void Decoder::decodeData( const std::string& msg, double& value )
{
    Data_Container data;
    data.ParseFromString( msg );
    value = data.numeric();
}

void Decoder::decodeData( const std::string& msg, co::int32& value )
{
    Data_Container data;
    data.ParseFromString( msg );
    value = static_cast<co::int32>( data.numeric() );
}

void Decoder::decodeData( const std::string& msg, std::string& value )
{
    Data_Container data;
    data.ParseFromString( msg );
    value = data.str();
}
    
void Decoder::decodeData( const std::string& msg, co::IType* descriptor, co::Any& value )
{
    Argument arg;
    arg.ParseFromString( msg );
    PBArgToAny( arg, descriptor, value );
}

    
}