#include "Encoder.h"

#include "Message.pb.h"
#include "network/Connection.h"

#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>

namespace reef
{
   
    // ------------ Any to Protobuf conversion functions --------------- //
    
// Specializes for each Data container's different set function.
template <typename T>
static void setPBContainerData( Data_Container* container, T value ) 
{
    container->set_numeric( static_cast<double>( value ) );
}

template <>
void setPBContainerData<bool>( Data_Container* container, bool value ) 
{
    container->set_boolean( value );
}

template <>
void setPBContainerData<const std::string&>( Data_Container* container, const std::string& value ) 
{
    container->set_str( value );
}

// Extracts the provided type's data from Any (deals with arrays and values)
template <typename T>
static void anyWithTypeToPBArg( const co::Any& any, Argument* arg )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<T>( container, any.get<T>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Argument
    const co::Range<const T> range = any.get<const co::Range<const T> >();
    
    size_t size = range.getSize();
    for( int i = 0; i < size; i++ )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<T>( container, range[i] );
    }
}

template <>
void anyWithTypeToPBArg<std::string>( const co::Any& any, Argument* arg )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<const std::string&>( container, any.get<const std::string&>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Argument
    const co::Range<const std::string> range = any.get<const co::Range<const std::string> >();
    
    size_t size = range.getSize();
    for( int i = 0; i < size; i++ )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<const std::string&>( container, range[i] );
    }
}

template <>
void anyWithTypeToPBArg<bool>( const co::Any& any, Argument* arg )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<bool>( container, any.get<bool>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Argument
    const std::vector<bool>& vec = any.get<const std::vector<bool> &>();
    
    size_t size = vec.size();
    for( int i = 0; i < size; i++ )
    {
        Data_Container* container = arg->add_data();
        setPBContainerData<bool>( container, vec[i] );
    }
}

// Converts an any containing a vlue type to a protobuf Argument
void anyToPBArg( const co::Any& any, Argument* arg )
{
    std::vector<co::Any> anyVec;
    co::TypeKind kind = any.getKind();
    
    if( kind == co::TK_ARRAY )
        kind = any.getType()->getKind();
    
    switch( kind )
    {
        case co::TK_BOOLEAN:
            anyWithTypeToPBArg<bool>( any, arg );
            break;
        case co::TK_INT8:
            anyWithTypeToPBArg<co::int8>( any, arg );
            break;
        case co::TK_UINT8:
            anyWithTypeToPBArg<co::uint8>( any, arg );
            break;
        case co::TK_INT16:
            anyWithTypeToPBArg<co::int16>( any, arg );
            break;
        case co::TK_UINT16:
            anyWithTypeToPBArg<co::uint16>( any, arg );
            break;
        case co::TK_INT32:
            anyWithTypeToPBArg<co::int32>( any, arg );
            break;
        case co::TK_UINT32:
            anyWithTypeToPBArg<co::uint32>( any, arg );
            break;
        case co::TK_INT64:
            anyWithTypeToPBArg<co::int64>( any, arg );
            break;
        case co::TK_UINT64:
            anyWithTypeToPBArg<co::uint64>( any, arg );
            break;
        case co::TK_FLOAT:
            anyWithTypeToPBArg<float>( any, arg );
            break;
        case co::TK_DOUBLE:
            anyWithTypeToPBArg<double>( any, arg );
            break;
        case co::TK_STRING:
            anyWithTypeToPBArg<std::string>( any, arg );
            break;
        default:
            assert( false );
    }
}

Encoder::Encoder()
{
    _message = new Message();
}

Encoder::~Encoder()
{
    delete _message;
}
    
void Encoder::encodeNewInstMsg( const std::string& typeName, std::string& msg )
{
    _message->set_instance_id( 0 ); // 0 is always the node channel
    _message->set_has_return( true );

    
	Message_New_Inst* msgNewInst = _message->mutable_msg_new_inst();
    msgNewInst->set_new_instance_type( typeName );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Encoder::encodeAccessInstMsg( co::int32 instanceID, bool increment, std::string& msg )
{
    _message->set_instance_id( 0 ); // 0 is always the node channel
    _message->set_has_return( false );
    
    
	Message_Acc_Inst* msgAccInst = _message->mutable_msg_acc_inst();
    //TODO: set referer as self
    msgAccInst->set_increment( increment );
    msgAccInst->set_instance_id( instanceID );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}
    
void Encoder::beginEncodingCallMsg( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx,
                                   bool hasReturn )
{
    if( instanceID == 0 )
        throw new co::Exception( "A call msg can't have an instanceID of 0" );
    
    _message->set_instance_id( instanceID );
    _message->set_has_return( hasReturn );
    
    _msgMember = _message->mutable_msg_member();
    _msgMember->set_facet_idx( facetIdx );
    _msgMember->set_member_idx( memberIdx );
}

void Encoder::addValueParam( const co::Any& param )
{
    checkIfCallMsg();
    Argument* PBArg = _msgMember->add_arguments();
    anyToPBArg( param, PBArg );
}

void Encoder::addRefParam( co::int32 instanceID, co::int32 facetIdx, RefOwner owner, 
                        const std::string* ownerAddress )
{
    checkIfCallMsg();
    Argument* PBArg = _msgMember->add_arguments();
    Data_Container* dc = PBArg->add_data();
    Ref_Type* refType = dc->mutable_ref_type();
    
    refType->set_instance_id( instanceID );
    refType->set_facet_idx( facetIdx );
    

    switch( owner )
    {
    case RefOwner::LOCAL:
            refType->set_owner( Ref_Type::OWNER_LOCAL );
            break;
    case RefOwner::RECEIVER:
            refType->set_owner( Ref_Type::OWNER_RECEIVER );
            break;
    case RefOwner::ANOTHER:
            refType->set_owner( Ref_Type::OWNER_ANOTHER );
            refType->set_owner_ip( *ownerAddress );
            break;        
    }
}


void Encoder::finishEncodingCallMsg( std::string& msg )
{
    _message->SerializeToString( &msg );
    _message->Clear();
    _msgMember = 0;
}

void Encoder::encodeData( bool value, std::string& msg )
{
    Data_Container data;
    data.set_boolean( value );
    data.SerializeToString( &msg );
}

void Encoder::encodeData( double value, std::string& msg )
{
    Data_Container data;
    data.set_numeric( value );
    data.SerializeToString( &msg );
}

void Encoder::encodeData( co::int32 value, std::string& msg )
{
    Data_Container data;
    data.set_numeric( static_cast<co::int32>( value ) );
    data.SerializeToString( &msg );
}

void Encoder::encodeData( const std::string& value, std::string& msg )
{
    Data_Container data;
    data.set_str( value );
    data.SerializeToString( &msg );
}
    
void Encoder::encodeData( const co::Any& value, std::string& msg )
{
    Argument returnArg;
    anyToPBArg( value, &returnArg );
    returnArg.SerializeToString( &msg );
}
    
void Encoder::checkIfCallMsg()
{
    if( !_msgMember )
        throw new co::Exception( "Could not add a Parameter to an empty Message" );
}
    
}