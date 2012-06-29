#include "Marshaller.h"

#include "Message.pb.h"

#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>
#include <co/IReflector.h>
#include <co/IRecordType.h>

namespace reef {
namespace rpc {
  
// ------------ Any to Protobuf conversion functions --------------- //

// Specializes for each Data container's different set function.
template <typename T>
static void setPBContainerData( Any_PB* container, T value ) 
{
    container->set_numeric( static_cast<double>( value ) );
}

template <>
void setPBContainerData<bool>( Any_PB* container, bool value ) 
{
    container->set_boolean( value );
}

template <>
void setPBContainerData<const std::string&>( Any_PB* container, const std::string& value ) 
{
    container->set_str( value );
}

// Extracts the provided type's data from Any (deals with arrays and values)
template <typename T>
static void anyWithTypeToPBParam( const co::Any& any, Parameter* param )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<T>( container, any.get<T>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Parameter
    const co::Range<const T> range = any.get<const co::Range<const T> >();
    
    size_t size = range.getSize();
    for( int i = 0; i < size; i++ )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<T>( container, range[i] );
    }
}

template <>
void anyWithTypeToPBParam<std::string>( const co::Any& any, Parameter* param )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<const std::string&>( container, any.get<const std::string&>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Parameter
    const co::Range<const std::string> range = any.get<const co::Range<const std::string> >();
    
    size_t size = range.getSize();
    for( int i = 0; i < size; i++ )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<const std::string&>( container, range[i] );
    }
}

template <>
void anyWithTypeToPBParam<bool>( const co::Any& any, Parameter* param )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<bool>( container, any.get<bool>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Parameter
    const std::vector<bool>& vec = any.get<const std::vector<bool> &>();
    
    size_t size = vec.size();
    for( int i = 0; i < size; i++ )
    {
        Any_PB* container = param->add_any();
        setPBContainerData<bool>( container, vec[i] );
    }
}

void complexToPBParam( const co::Any& instance, Parameter* complexParam );

// Converts an any containing a vlue type to a protobuf Parameter
void valueToPBParam( const co::Any& any, Parameter* param )
{
    std::vector<co::Any> anyVec;
    co::TypeKind kind = any.getKind();
    
    if( kind == co::TK_ARRAY )
        kind = any.getType()->getKind();
    
    
    switch( kind )
    {
        case co::TK_BOOLEAN:
            anyWithTypeToPBParam<bool>( any, param );
            break;
        case co::TK_INT8:
            anyWithTypeToPBParam<co::int8>( any, param );
            break;
        case co::TK_UINT8:
            anyWithTypeToPBParam<co::uint8>( any, param );
            break;
        case co::TK_INT16:
            anyWithTypeToPBParam<co::int16>( any, param );
            break;
        case co::TK_UINT16:
            anyWithTypeToPBParam<co::uint16>( any, param );
            break;
        case co::TK_INT32:
            anyWithTypeToPBParam<co::int32>( any, param );
            break;
        case co::TK_UINT32:
            anyWithTypeToPBParam<co::uint32>( any, param );
            break;
        case co::TK_INT64:
            anyWithTypeToPBParam<co::int64>( any, param );
            break;
        case co::TK_UINT64:
            anyWithTypeToPBParam<co::uint64>( any, param );
            break;
        case co::TK_FLOAT:
            anyWithTypeToPBParam<float>( any, param );
            break;
        case co::TK_DOUBLE:
            anyWithTypeToPBParam<double>( any, param );
            break;
        case co::TK_STRING:
            anyWithTypeToPBParam<std::string>( any, param );
            break;
        case co::TK_STRUCT:
        case co::TK_NATIVECLASS:
            complexToPBParam( any, param );
            break;
        case co::TK_ANY:
        {
            const co::Any& internalAny =  any.get<const co::Any&>();
            param->set_any_type( static_cast<co::uint32>( internalAny.getKind() ) );
            valueToPBParam( internalAny, param );
            break;
        }
        default:
            assert( false );
    }
}


void complexToPBParam( const co::Any& instance, Parameter* complexParam )
{
    assert( instance.getKind() != co::TK_ARRAY );
    
    co::IRecordType* rt = co::cast<co::IRecordType>( instance.getType() );
    co::IReflector* refl = rt->getReflector();
    Any_PB* container = complexParam->add_any();
    Complex_Type* complexType = container->mutable_complex_type();
    
    for( co::Range<co::IField* const> fields = rt->getFields(); fields; fields.popFirst() )
    {
        Parameter* fieldArg = complexType->add_field();
        co::Any fieldAny;
        refl->getField( instance, fields.getFirst(), fieldAny );
        valueToPBParam( fieldAny, fieldArg );
    }
}
    
void refToPBParam( const ReferenceType& refType, Parameter* refParam )
{
    Any_PB* PBAny = refParam->add_any();
    Ref_Type* ref_type = PBAny->mutable_ref_type();
    
    ref_type->set_instance_id( refType.instanceID );
    ref_type->set_facet_idx( refType.facetIdx );
    
    switch( refType.owner )
    {
        case OWNER_SENDER:
            ref_type->set_owner( Ref_Type::OWNER_SENDER );
            ref_type->set_instance_type( refType.instanceType );
            ref_type->set_owner_endpoint( refType.ownerEndpoint );
            break;
        case OWNER_RECEIVER:
            ref_type->set_owner( Ref_Type::OWNER_RECEIVER );
            break;
        case OWNER_ANOTHER:
            ref_type->set_owner( Ref_Type::OWNER_ANOTHER );
            ref_type->set_instance_type( refType.instanceType );
            ref_type->set_owner_endpoint( refType.ownerEndpoint );
            break;        
    }
}

ParameterPusher::ParameterPusher()
{
}

void ParameterPusher::pushValue( const co::Any& valueType )
{
    assert( _invocation );
    
    Parameter* PBParam = _invocation->add_params();
    valueToPBParam( valueType, PBParam );
}

void ParameterPusher::pushReference( const ReferenceType& refType )
{
    assert( _invocation );
    
    Parameter* PBParam = _invocation->add_params();
    refToPBParam( refType, PBParam );
}

Marshaller::Marshaller() : _request( 0 ), _invocation( 0 ), _msgClear( true )
{
    _message = new Message();
}

Marshaller::~Marshaller()
{
    delete _message;
}

void Marshaller::marshalNew( inString requesterEndpoint, inString instanceType, outString msg )
{
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::REQUEST_NEW );
    
    Request* request = _message->mutable_request();
    request->set_new_instance_type( instanceType );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalLookup( inString requesterEndpoint, inString lookupKey, outString msg )
{
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::REQUEST_LOOKUP );
    
    Request* request = _message->mutable_request();
    request->set_lookup_key( lookupKey );
    
    _message->SerializeToString( &msg );
    _message->Clear();
    _request = 0;
}

void Marshaller::marshalLease( inString requesterEndpoint, co::int32 leaseInstanceID, outString msg )
{
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::REQUEST_LEASE );
    
    Request* request = _message->mutable_request();
    request->set_lease_instance_id( leaseInstanceID );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalCancelLease( inString requesterEndpoint, co::int32 leaseInstanceID, 
                                    outString msg )
{
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::REQUEST_CANCEL_LEASE );
    
    Request* request = _message->mutable_request();
    request->set_lease_instance_id( leaseInstanceID );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}
    
ParameterPusher& Marshaller::beginInvocation( inString requesterEndpoint, InvocationDetails details )
                                                   
{   
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::INVOCATION );
    
    Invocation* invocation = _message->mutable_invocation();
    invocation->set_instance_id( details.instanceID );
    invocation->set_facet_idx( details.facetIdx );
    invocation->set_member_idx( details.memberIdx );
    invocation->set_type_depth( details.typeDepth );
    invocation->set_synch( details.hasReturn );
    
    _paramPusher._invocation = invocation;
    return _paramPusher;
    
    _msgClear = false;
}

void Marshaller::marshalInvocation( outString msg )
{
    assert( _paramPusher._invocation );
    
    _message->SerializeToString( &msg );
    _message->Clear();
    
    _paramPusher._invocation = 0;
    _msgClear = true;
}
    
void Marshaller::marshalIntReturn( co::int32 value, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::RETURN );
    
    _message->set_ret_int( value );
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalValueTypeReturn( const co::Any& valueAny, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::RETURN );
    
    Parameter* param = _message->mutable_ret_value();
    valueToPBParam( valueAny, param );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalRefTypeReturn( const ReferenceType& refType, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::RETURN );
    
    Parameter* param = _message->mutable_ret_value();
    refToPBParam( refType, param );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

}
}