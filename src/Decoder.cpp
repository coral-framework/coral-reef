#include "Decoder.h"

#include "Message.pb.h"
#include "network/Connection.h"

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

Decoder::Decoder() : _currentParam( 0 )
{
    _message = new Message();
}

Decoder::~Decoder()
{
    delete _message;
}
    
// sets the message that will be decoded and return its type and destination
void Decoder::setMsgForDecoding( const std::string& msg, co::int32& instanceID )
{
    _message->Clear();
    _message->ParseFromString( msg );
    instanceID = _message->instance_id();
}

// if msg type is NEW, then, this function will decode it
void Decoder::decodeNewInstMsg( std::string& typeName )
{
    const Message_New& msgNew = _message->msg_new();
    typeName = msgNew.component_type_name();
}

/* 
 Starts a decoding state of call/field msg. 
 The decoding state will only be reset after all params are decoded.
 */
void Decoder::beginDecodingCallMsg( co::int32& facetIdx, co::int32& memberIdx )
{
    _msgMember = &_message->msg_member();
    _currentParam = 0;
    facetIdx = _msgMember->facet_idx();
    memberIdx = _msgMember->member_idx();
}

void Decoder::getValueParam( co::Any& param, co::IType* descriptor )
{
    checkIfCallMsg();
    PBArgToAny( _msgMember->arguments( _currentParam++ ), descriptor, param );
}

void Decoder::getRefParam( co::int32& instanceID, co::int32& facetIdx, RefOwner& owner,
                 std::string& ownerAddress )
{
    checkIfCallMsg();
    const Ref_Type& refType = _msgMember->arguments( _currentParam++ ).data( 0 ).ref_type();
    instanceID = refType.instance_id();
    facetIdx = refType.facet_idx();
    
    switch( refType.owner() )
    {
        case Ref_Type::OWNER_LOCAL:
            owner = RefOwner::LOCAL;
            return;
        case Ref_Type::OWNER_RECEIVER:
            owner = RefOwner::RECEIVER;
            return;
        case Ref_Type::OWNER_ANOTHER:
            owner = RefOwner::ANOTHER;
            ownerAddress = refType.owner_ip();
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

void Decoder::checkIfCallMsg()
{
    if( !_msgMember )
        throw new co::Exception( "Requires a call message being decoded to extract Param" );
}
    
}