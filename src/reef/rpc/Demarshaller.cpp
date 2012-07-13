#include "Demarshaller.h"

#include "Message.pb.h"

#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>
#include <co/IReflector.h>
#include <co/IRecordType.h>

#include <sstream>

namespace reef {
namespace rpc {

    // -------------- Protobuf to Any conversion functions ------------------//

// Specializes for each Data container's different get function.
template <typename T>
static T getPBContainerData( const Any_PB& container )
{
    if( !container.has_numeric() )
        throw std::string( "No numeric data in parameter" );
    
    return static_cast<T>( container.numeric() );
}

// ------------- get and set functions specialization for string and bool ----------- //
template <>
const std::string& getPBContainerData<const std::string&>( const Any_PB& container )
{
    if( !container.has_str() )
        throw std::string( "No string data in parameter" );
    
    return container.str();
}

template <>
bool getPBContainerData<bool>( const Any_PB& container )
{
    if( !container.has_boolean() )
        throw std::string( "No boolean data in parameter" );
    
    return container.boolean();
}
    
// Extracts the provided type's data from Parameter (deals with arrays and values)
template <typename T>
static void PBParamWithTypeToAny( const Parameter& param, co::Any& any, co::IType* elementType )
{
    if( !elementType )
    {
        any.set<T>( getPBContainerData<T>( param.any( 0 ) ) );
        return;
    }
    
    size_t size = param.any().size();
    if( size == 0 ) // required for vector subscript out of range assertion
        return;
    
    std::vector<co::uint8>& vec = any.createArray( elementType, size );
    T* toCast = reinterpret_cast<T*>( &vec[0] );
    for( int i = 0; i < size; i++ )
    {
        toCast[i] = getPBContainerData<T>( param.any( i ) );
    }
}

// ----------------- PBParamWithTypeToAny specializations for string and bool --------------- //
template <>
void PBParamWithTypeToAny<std::string>( const Parameter& param, co::Any& any, co::IType* elementType )
{
    if( !elementType )
    {
        std::string& anyString = any.createString();
        anyString = getPBContainerData<const std::string&>( param.any( 0 ) );
        return;
    }
    
    size_t size = param.any().size();
    if( size == 0 ) // required for vector subscript out of range assertion
        return;
    
    std::vector<co::uint8>& vec = any.createArray( elementType, size );
    std::string* toCast = reinterpret_cast<std::string*>( &vec[0] );
    for( int i = 0; i < size; i++ )
    {
        toCast[i] = getPBContainerData<const std::string&>( param.any( i ) );
    }
}

// Will be removed with all the coral 0.7 workarounds
co::IType* kind2Type( co::TypeKind kind )
{
    switch( kind )
    {
        case co::TK_BOOLEAN:
            return co::getType( "bool" );
        case co::TK_INT32:
            return co::getType( "int32" );
        case co::TK_UINT32:
            return co::getType( "uint32" );
        case co::TK_FLOAT:
            return co::getType( "float" );
        case co::TK_DOUBLE:
            return co::getType( "double" );
        case co::TK_STRING:
            return co::getType( "string" );
        default:
            throw std::string( "Only co::Any containing simple value types are supported" );
    }
    return 0;
}
 
void PBParamToComplex( const Parameter& param, co::IType* descriptor, co::Any& complexAny );
    
void PBParamToAny( const Parameter& param, co::IType* descriptor, co::Any& any )
{
    co::TypeKind kind = descriptor->getKind();
    co::IType* elementType = 0; // only used for arrays
    
    if( kind == co::TK_ARRAY )
    {
        elementType = co::cast<co::IArray>( descriptor )->getElementType();
        kind = elementType->getKind();
    }
    else
    {
        if( param.any().size() < 1 )
            throw std::string( "Empty parameter" );
    }
    
    switch( kind )
    {
        case co::TK_BOOLEAN:
            PBParamWithTypeToAny<bool>( param, any, elementType );
            break;
        case co::TK_INT8:
            PBParamWithTypeToAny<co::int8>( param, any, elementType );
            break;
        case co::TK_UINT8:
            PBParamWithTypeToAny<co::uint8>( param, any, elementType );
            break;
        case co::TK_INT16:
            PBParamWithTypeToAny<co::int16>( param, any, elementType );
            break;
        case co::TK_UINT16:
            PBParamWithTypeToAny<co::uint16>( param, any, elementType );
            break;
        case co::TK_INT32:
            PBParamWithTypeToAny<co::int32>( param, any, elementType );
            break;
        case co::TK_UINT32:
            PBParamWithTypeToAny<co::uint32>( param, any, elementType );
            break;
        case co::TK_INT64:
            PBParamWithTypeToAny<co::int64>( param, any, elementType );
            break;
        case co::TK_UINT64:
            PBParamWithTypeToAny<co::uint64>( param, any, elementType );
            break;
        case co::TK_FLOAT:
            PBParamWithTypeToAny<float>( param, any, elementType );
            break;
        case co::TK_DOUBLE:
            PBParamWithTypeToAny<double>( param, any, elementType );
            break;
        case co::TK_STRING:
            PBParamWithTypeToAny<std::string>( param, any, elementType );
            break;
        case co::TK_STRUCT:
        case co::TK_NATIVECLASS:
            PBParamToComplex( param, descriptor, any );
            break;
        case co::TK_ANY:
        {
            static co::Any internalAny; // TODO remove
            PBParamToAny( param, kind2Type( static_cast<co::TypeKind>( param.any_type() ) ), internalAny );
            any.set<const co::Any&>( internalAny );
            break;
        }
        default:
        {
            if( elementType )
                throw std::string( elementType->getFullName() ).append( " array is not supported" );
        }
    }
}
    
void PBParamToComplex( const Parameter& param, co::IType* descriptor, co::Any& complexAny )
{
    assert( descriptor->getKind() != co::TK_ARRAY );
    
    complexAny.createComplexValue( descriptor );
    co::IRecordType* rt = co::cast<co::IRecordType>( descriptor );
    co::IReflector* refl = rt->getReflector();
    
    const Any_PB& container = param.any( 0 );
    
    if( !container.has_complex_type() )
        throw std::string( "No complex type data in parameter" );
    
    const Complex_Type& complex = container.complex_type();
    
    co::Range<co::IField* const> fields = rt->getFields();
    co::int32 fieldCount = fields.getSize();
    for( co::int32 i = 0; i < fieldCount; i++ )
    {
        co::IField* field = fields[i];
        const Parameter& fieldArg = complex.field( i );
        co::Any fieldAny;
        PBParamToAny( fieldArg, field->getType(), fieldAny );
        refl->setField( complexAny, field, fieldAny );
    }
}
    
void ParameterPuller::pullValue( co::IType* descriptor, co::Any& valueType )
{
    try
    {
        PBParamToAny( _invocation->params( _currentParam ), descriptor, valueType );
        _currentParam++;
    }
    catch( std::string& e )
    {
        CORAL_THROW( RemotingException, "Error in Invocation in the reading of parameter " << 
                    _currentParam << ": " << e );        
    }
}
    
void ParameterPuller::pullReference( ReferenceType& refType )
{
    const Parameter& param = _invocation->params( _currentParam );
    
    size_t paramSize = param.any().size();
    if( paramSize < 1 )
    {
        CORAL_THROW( RemotingException, "Error in Invocation in the reading of parameter " << 
                    _currentParam << ": " << "Empty parameter" );
    }
    else if( paramSize > 1 )
    {
        CORAL_THROW( RemotingException, "Error in Invocation in the reading of parameter " << 
                _currentParam << ": " << "Multiple references stored in a single value parameter" );
    }
    
    const Ref_Type& ref_type = param.any( 0 ).ref_type();
    
    if( !ref_type.IsInitialized() )
    {
        CORAL_THROW( RemotingException, "Error in Invocation in the reading of parameter " << 
                    _currentParam << ": " << "Missing fields of the reference type" );
    }
    
    refType.instanceID = ref_type.instance_id();
    refType.facetIdx = ref_type.facet_idx();
    
    switch( ref_type.owner() )
    {
        case Ref_Type::OWNER_SENDER:
            refType.owner = OWNER_SENDER;
            refType.ownerEndpoint = ref_type.owner_endpoint();
            refType.instanceType = ref_type.instance_type();
            break;
        case Ref_Type::OWNER_RECEIVER:
            refType.owner = OWNER_RECEIVER;
            break;
        case Ref_Type::OWNER_ANOTHER:
            refType.owner = OWNER_ANOTHER;
            refType.ownerEndpoint = ref_type.owner_endpoint();
            refType.instanceType = ref_type.instance_type();
            break;
        default:
            assert( false );
    }

    _currentParam++;
}
 
void ParameterPuller::setInvocation( const Invocation* invocation )
{
    _invocation = invocation;
    _currentParam = 0;
}
    
ParameterPuller::ParameterPuller() : _currentParam( 0 ), _invocation( 0 )
{
}
    
Demarshaller::Demarshaller() : _currentParam( 0 )
{
    _message = new Message();
    _parameter = new Parameter();
}

Demarshaller::~Demarshaller()
{
    delete _message;
    delete _parameter;
}
    
// sets the message that will be decoded and return its type and destination
MessageType Demarshaller::demarshal( inString data )
{
    _message->Clear();
    _message->ParseFromString( data );
    
    if( !_message->IsInitialized() )
        CORAL_THROW( RemotingException, "Invalid message data" );
    
    _msgType = INVALID;
    
    switch( _message->type() )
    {
        case Message::INVOCATION:
        {
            if( _message->has_invocation() )
                _msgType = INVOCATION;
            
            break;
        }
        case Message::RETURN:
        {
            if( _message->has_ret_int() || _message->has_ret_value() )
                _msgType = RETURN;
            
            break;
        }
        case Message::REQUEST_NEW:
        {
            if( _message->has_request() )
                _msgType = REQUEST_NEW;
            
            break;
        }
        case Message::REQUEST_LOOKUP:
        {
            if( _message->has_request() )
                _msgType = REQUEST_LOOKUP;
            
            break;
        }
        case Message::REQUEST_LEASE:
        {
            if( _message->has_request() )
                _msgType = REQUEST_LEASE;
            
            break;
        }
        case Message::REQUEST_CANCEL_LEASE:
        {
            if( _message->has_request() )
                _msgType = REQUEST_CANCEL_LEASE;
            
            break;
        }
        case Message::EXCEPTION:
        {
            if( _message->has_exception() )
                _msgType = EXCEPTION;
            
            break;
        }
    }
    
    if( _msgType == INVALID )
        CORAL_THROW( RemotingException, "Invalid message data" );
    
    return _msgType;
}

// if msg type is NEW, then, this function will decode it
void Demarshaller::getNew( outString requesterEndpoint, outString instanceType )
{
    assert( _msgType == REQUEST_NEW );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Request& request = _message->request();
    
    if( !request.IsInitialized() || !request.has_new_instance_type() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    instanceType = request.new_instance_type();
}

void Demarshaller::getLookup( outString requesterEndpoint, outString lookupKey )
{
    assert( _msgType == REQUEST_LOOKUP );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Request& request = _message->request();
    
    if( !request.IsInitialized() || !request.has_lookup_key() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    lookupKey = request.lookup_key();
}
    
void Demarshaller::getLease( outString requesterEndpoint, co::int32& leaseInstanceID )
{
    assert( _msgType == REQUEST_LEASE || _msgType == REQUEST_CANCEL_LEASE );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Request& request = _message->request();
    
    if( !request.IsInitialized() || !request.has_lease_instance_id() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    leaseInstanceID = request.lease_instance_id();
}
   
void Demarshaller::getCancelLease( outString requesterEndpoint, co::int32& leaseInstanceID )
{
    getLease( requesterEndpoint, leaseInstanceID );
}
    
/* 
 Starts a decoding state of call/field msg. 
 The decoding state will only be reset after all params are decoded.
 */
ParameterPuller& Demarshaller::getInvocation( outString requesterEndpoint, 
                                             InvocationDetails& details )
{
    assert( _msgType == INVOCATION );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Invocation& invocation = _message->invocation();
    
    if( !invocation.IsInitialized() )
        CORAL_THROW( RemotingException, "Invalid invocation data" );
    
    details.instanceID = invocation.instance_id();
    details.facetIdx = invocation.facet_idx();
    details.memberIdx = invocation.member_idx();
    details.typeDepth = invocation.type_depth();
    details.hasReturn = invocation.synch();
    
    _puller.setInvocation( &invocation );
    
    return _puller;
}

void Demarshaller::getValueTypeReturn( co::IType* descriptor, co::Any& valueAny )
{
    assert( _msgType == RETURN );
    
    const Parameter& returnValue = _message->ret_value();

    try
    {
        PBParamToAny( returnValue, descriptor, valueAny );
    }
    catch( std::string& e )
    {
        CORAL_THROW( RemotingException, "Error converting return value: " << e );        
    }
}
    
void Demarshaller::getRefTypeReturn( ReferenceType& refType )
{    
    assert( _msgType == RETURN );
    
    const Parameter& PBParam = _message->ret_value();
    
    size_t paramSize = PBParam.any().size();
    if( paramSize < 1 )
    {
        CORAL_THROW( RemotingException, "Error converting return reference : Empty parameter" );
    }
    else if( paramSize > 1 )
    {
        CORAL_THROW( RemotingException, "Error converting return reference : Multiple references" );
    }
    
    const Ref_Type& ref_type = PBParam.any( 0 ).ref_type();
    
    if( !ref_type.IsInitialized() )
    {
        CORAL_THROW( RemotingException, "Error converting return reference : Missing fields" );
    }

    refType.instanceID = ref_type.instance_id();
    refType.facetIdx = ref_type.facet_idx();
    
    switch( ref_type.owner() )
    {
        case Ref_Type::OWNER_SENDER:
            refType.owner = OWNER_SENDER;
            refType.ownerEndpoint = ref_type.owner_endpoint();
            refType.instanceType = ref_type.instance_type();
            return;
        case Ref_Type::OWNER_RECEIVER:
            refType.owner = OWNER_RECEIVER;
            return;
        case Ref_Type::OWNER_ANOTHER:
            refType.owner = OWNER_ANOTHER;
            refType.ownerEndpoint = ref_type.owner_endpoint();
            refType.instanceType = ref_type.instance_type();
            return;
    }
}
 
co::int32 Demarshaller::getIntReturn()
{
    assert( _msgType == RETURN );
    return _message->ret_int();
}
    
ExceptionType Demarshaller::getException( outString exTypeName, outString what )
{
    assert( _msgType == EXCEPTION );
    const Exception& ex = _message->exception();
    
    if( !ex.IsInitialized() )
        CORAL_THROW( RemotingException, "Invalid exception data" );
    
    exTypeName = ex.type_name();
    what = ex.what();
    
    switch( ex.type() )
    {
        case Exception::CORAL:
            return EX_CORAL;
        case Exception::REMOTING:
            return EX_REMOTING;
        case Exception::STD:
            return EX_STD;
    }
    
}
    
}
    
}