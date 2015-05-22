#include "Marshaller.h"

#include "Message.pb.h"

#include <co/Log.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>
#include <co/IReflector.h>
#include <co/IRecordType.h>

#include <sstream>

namespace rpc {
  
// ------------ Any to Protobuf conversion functions --------------- //

// Specializes for each Data container's different set function.
template <typename T>
static void setPBContainerData( Container* container, T value ) 
{
    container->set_numeric( static_cast<double>( value ) );
}

template <>
void setPBContainerData<bool>( Container* container, bool value ) 
{
    container->set_boolean( value );
}

template <>
void setPBContainerData<const std::string&>( Container* container, const std::string& value ) 
{
    container->set_str( value );
}

// Extracts the provided type's data from Any (deals with arrays and values)
template <typename T>
static void simpleToPBParam( const co::Any& any, co::IType* descriptor, Parameter* param )
{
    co::TypeKind kind = descriptor->getKind();
    
    // if the Any is a single value, set it directly 
    if( kind != co::TK_ARRAY )
    {
        Container* container = param->add_container();
        setPBContainerData<T>( container, any.get<T>() );
        return;
    }
    
    size_t size = any.getCount();
    for( int i = 0; i < size; i++ )
    {
        Container* container = param->add_container();
        setPBContainerData<T>( container, any[i].get<T>() );
    }
}
    
void complexToPBParam( const co::Any& complex, co::IType* descriptor, Parameter* complexParam );
void anyToPBParam( const co::Any& any, co::IType* descriptor, Parameter* param );
    
// Converts an any containing a vlue type to a protobuf Parameter
void valueToPBParam( const co::Any& any, co::IType* descriptor, Parameter* param )
{
	CORAL_DLOG( INFO ) << "valueToPBParam " << descriptor->getFullName() << " " << descriptor->getKind();
    co::TypeKind kind = descriptor->getKind();
    
    if( kind == co::TK_ARRAY )
    {
        co::IType* elementType = co::cast<co::IArray>( descriptor )->getElementType();
        kind = elementType->getKind();
        
        if( kind == co::TK_ANY )
            CORAL_THROW( RemotingException, "Arrays of co.Any not supported yet" );

		CORAL_DLOG( INFO ) << "valueToPBParam array " << elementType->getFullName() << " " << kind;
    }
    
    switch( kind )
    {
        case co::TK_BOOL:
            simpleToPBParam<bool>( any, descriptor, param );
            break;
        case co::TK_INT8:
            simpleToPBParam<co::int8>( any, descriptor, param );
            break;
        case co::TK_UINT8:
            simpleToPBParam<co::uint8>( any, descriptor, param );
            break;
        case co::TK_INT16:
            simpleToPBParam<co::int16>( any, descriptor, param );
            break;
        case co::TK_UINT16:
            simpleToPBParam<co::uint16>( any, descriptor, param );
            break;
        case co::TK_INT32:
            simpleToPBParam<co::int32>( any, descriptor, param );
            break;
        case co::TK_UINT32:
            simpleToPBParam<co::uint32>( any, descriptor, param );
            break;
		case co::TK_ENUM:
            simpleToPBParam<co::uint32>( any, descriptor, param );
            break;
        case co::TK_FLOAT:
            simpleToPBParam<float>( any, descriptor, param );
            break;
        case co::TK_DOUBLE:
            simpleToPBParam<double>( any, descriptor, param );
            break;
        case co::TK_STRING:
            simpleToPBParam<const std::string&>( any, descriptor, param );
            break;
        case co::TK_STRUCT:
        case co::TK_NATIVECLASS:
            complexToPBParam( any, descriptor, param );
            break;
        case co::TK_ANY:
            anyToPBParam( any, descriptor, param );
            break;
        default:
            assert( false );
    }
}

void anyToPBParam( const co::Any& any, co::IType* descriptor, Parameter* param )
{
    Container* container = param->add_container();
    Any_Type* any_type = container->mutable_any_type();
    
    co::Any asIn = any.asIn();
    
    co::TypeKind internalKind = asIn.getKind();

	CORAL_DLOG( INFO ) << "any asIn getKind " << asIn.getKind();
    
    if( internalKind == co::TK_INTERFACE )
        CORAL_THROW( RemotingException, "interfaces inside anys nyi" );
    
	CORAL_DLOG( INFO ) << "internalKind marshaller " << internalKind;
    any_type->set_kind( internalKind );
    
	if( internalKind == co::TK_STRUCT || internalKind == co::TK_NATIVECLASS || internalKind == co::TK_ARRAY )
        any_type->set_type( asIn.getType()->getFullName() );
        
    if( internalKind != co::TK_NULL )
	{
		CORAL_DLOG( INFO ) << "from any";
        valueToPBParam( asIn, asIn.getType(), any_type->mutable_param() );
	}
}
 
void complexToPBParam( const co::Any& complex, co::IType* descriptor, Parameter* complexParam );
    
void complexArrayToPBParam( const co::Any& complexArray, co::IType* descriptor, 
                           Parameter* complexArrayParam )
{
    co::IType* elementsType = co::cast<co::IArray>( descriptor )->getElementType();
    co::int32 size = complexArray.getCount();
    for( int i = 0; i < size; i++ )
    {
        const co::Any& element = complexArray[i];
        complexToPBParam( element, elementsType, complexArrayParam );
    }
}
    
void complexToPBParam( const co::Any& complex, co::IType* descriptor, Parameter* complexParam )
{
    if( descriptor->getKind() == co::TK_ARRAY )
    {
        complexArrayToPBParam( complex.asIn(), descriptor, complexParam );
        return;
    }
    
    co::IRecordType* rt = co::cast<co::IRecordType>( descriptor );
    co::IReflector* refl = rt->getReflector();
    Container* container = complexParam->add_container();
    Complex_Type* complexType = container->mutable_complex_type();
    
    for( co::TSlice<co::IField*> fields = rt->getFields(); fields; fields.popFirst() )
    {
        co::IField* field = fields.getFirst();
        co::IType* fieldType = field->getType();
        Parameter* fieldArg = complexType->add_field();
        
        co::AnyValue av; co::Any fieldAny( av );
        
        refl->getField( complex, fields.getFirst(), fieldAny );
        valueToPBParam( fieldAny.asIn(), fieldType, fieldArg );
    }
}
    
void refToPBParam( const ReferenceType& refType, Parameter* refParam )
{
    Container* PBAny = refParam->add_container();
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

void ParameterPusher::pushValue( const co::Any& valueType, co::IType* descriptor )
{
    assert( _params );

    Parameter* PBParam = _params->Add();
    valueToPBParam( valueType, descriptor, PBParam );
}

void ParameterPusher::pushReference( const ReferenceType& refType )
{
    assert( _params );
    
    Parameter* PBParam = _params->Add();
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
    request->set_instance_type( instanceType );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalLookup( inString requesterEndpoint, inString lookupKey, inString instanceType,
                               outString msg )
{
    assert( _msgClear );
    
    _message->set_requester_endpoint( requesterEndpoint );
    _message->set_type( Message::REQUEST_LOOKUP );
    
    Request* request = _message->mutable_request();
    request->set_lookup_key( lookupKey );
    request->set_instance_type( instanceType );
    
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
    
	_paramPusher._params = invocation->mutable_params();
	_msgClear = false;
    return _paramPusher;
}

void Marshaller::marshalInvocation( outString msg )
{
    assert( _paramPusher._params && !_msgClear );
    
    _message->SerializeToString( &msg );
    _message->Clear();
    
    _paramPusher._params = 0;
    _msgClear = true;
}
 
ParameterPusher& Marshaller::beginOutput()
{
	assert( _msgClear );

    _message->set_type( Message::RETURN );
    
	_paramPusher._params = _message->mutable_output();
	_msgClear = false;
    return _paramPusher;
}

void Marshaller::marshalOutput( outString msg )
{
	 assert( _paramPusher._params && !_msgClear );
    
    _message->SerializeToString( &msg );
    _message->Clear();
    
    _paramPusher._params = 0;
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
    
void Marshaller::marshalException( ExceptionType exType, inString exTypeName, inString what, 
                                  outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::EXCEPTION );
    
    Exception* ex = _message->mutable_exception();
    switch( exType )
    {
        case EX_CORAL:
            ex->set_type( Exception::CORAL );
            break;
        case EX_REMOTING:
            ex->set_type( Exception::REMOTING );
            break;
        case EX_STD:
            ex->set_type( Exception::STD );
            break;
        case EX_UNKNOWN:
            ex->set_type( Exception::UNKNOWN );
            break;
    }
    ex->set_type_name( exTypeName );
    ex->set_what( what );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}

void Marshaller::marshalBarrierUp( inString requesterEndpoint, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::BARRIER_UP );
    _message->set_requester_endpoint( requesterEndpoint );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}
    
void Marshaller::marshalBarrierHit( inString requesterEndpoint, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::BARRIER_HIT );
    _message->set_requester_endpoint( requesterEndpoint );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}
    
void Marshaller::marshalBarrierDown( inString requesterEndpoint, outString msg )
{
    assert( _msgClear );
    
    _message->set_type( Message::BARRIER_DOWN );
    _message->set_requester_endpoint( requesterEndpoint );
    
    _message->SerializeToString( &msg );
    _message->Clear();
}
} // namespace rpc