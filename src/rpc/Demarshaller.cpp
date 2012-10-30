#include "Demarshaller.h"

#include "Message.pb.h"

#include <co/Log.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/Exception.h>
#include <co/IReflector.h>
#include <co/IRecordType.h>

#include <sstream>

namespace rpc {

    // -------------- Protobuf to Any conversion functions ------------------//

// Specializes for each Data container's different get function.
template <typename T>
static T getPBContainerData( const Container& container )
{
    if( !container.has_numeric() )
        throw std::string( "No numeric data in parameter" );
    
    return static_cast<T>( container.numeric() );
}

// ------------- get and set functions specialization for string and bool ----------- //
template <>
std::string getPBContainerData<std::string>( const Container& container )
{
    if( !container.has_str() )
        throw std::string( "No string data in parameter" );
    
    return std::string( container.str() );
}

template <>
bool getPBContainerData<bool>( const Container& container )
{
    if( !container.has_boolean() )
        throw std::string( "No boolean data in parameter" );
    
    return container.boolean();
}
    
// Extracts the provided type's data from Parameter (deals with arrays and values)
template <typename T>
static void PBParamWithTypeToAny( const Parameter& param, const co::Any& ret, co::IType* elementType )
{
    if( !elementType )
    {
        T value = getPBContainerData<T>( param.container( 0 ) );
        CORAL_LOG(INFO) << "Starting conversion of a basic type of value " << value;
        ret.put( value );
        return;
    }
    
    size_t size = param.container().size();
    if( size == 0 ) // required for vector subscript out of range assertion
        return;
    
	ret.resize( size );
    for( int i = 0; i < size; i++ )
    {
        ret[i].put( getPBContainerData<T>( param.container( i ) ) );
    }
}
    
// Will be removed with all the coral 0.7 workarounds
co::IType* kind2Type( co::TypeKind kind )
{
    switch( kind )
    {
        case co::TK_BOOL:
            return co::getType( "bool" );
        case co::TK_INT8:
            return co::getType( "int8" );
        case co::TK_UINT8:
            return co::getType( "uint8" );
        case co::TK_INT16:
            return co::getType( "int16" );
        case co::TK_UINT16:
            return co::getType( "uint16" );
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
            assert( false );
    }
    return 0;
}

void PBParamToComplex( const Parameter& param, co::IType* descriptor, const co::Any& ret );
void PBParamToAny( const Parameter& param, const co::Any& ret );

void PBParamToValue( const Parameter& param, co::IType* descriptor, const co::Any& ret )
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
        if( param.container().size() < 1 )
            throw std::string( "Empty parameter" );
    }
    
    switch( kind )
    {
        case co::TK_BOOL:
            PBParamWithTypeToAny<bool>( param, ret, elementType );
            break;
        case co::TK_INT8:
            PBParamWithTypeToAny<co::int8>( param, ret, elementType );
            break;
        case co::TK_UINT8:
            PBParamWithTypeToAny<co::uint8>( param, ret, elementType );
            break;
        case co::TK_INT16:
            PBParamWithTypeToAny<co::int16>( param, ret, elementType );
            break;
        case co::TK_UINT16:
            PBParamWithTypeToAny<co::uint16>( param, ret, elementType );
            break;
        case co::TK_INT32:
            PBParamWithTypeToAny<co::int32>( param, ret, elementType );
            break;
        case co::TK_UINT32:
            PBParamWithTypeToAny<co::uint32>( param, ret, elementType );
            break;
        case co::TK_FLOAT:
            PBParamWithTypeToAny<float>( param, ret, elementType );
            break;
        case co::TK_DOUBLE:
            PBParamWithTypeToAny<double>( param, ret, elementType );
            break;
        case co::TK_STRING:
            PBParamWithTypeToAny<std::string>( param, ret, elementType );
            break;
        case co::TK_STRUCT:
        case co::TK_NATIVECLASS:
            PBParamToComplex( param, descriptor, ret );
            break;
        case co::TK_ANY:
            PBParamToAny( param, ret );
        default:
        {
            if( elementType )
                throw std::string( elementType->getFullName() ).append( " array is not supported" );
        }
    }
}
    
void PBParamToAny( const Parameter& param, const co::Any& ret )
{
    const Any_Type& any_type = param.container( 0 ).any_type();
    co::TypeKind internalKind = static_cast<co::TypeKind>( any_type.kind() );
    
    CORAL_LOG(INFO) << "Starting conversion of a n Any of internalKind " << internalKind;
    
    if( internalKind == co::TK_NULL )
        return;
    
    co::IType* internalType;
    if( internalKind == co::TK_STRUCT || internalKind == co::TK_NATIVECLASS )
        internalType = co::getType( any_type.type() );
    else
        internalType = kind2Type( internalKind );
    
    ret.get<co::AnyValue&>().create( internalType );
    PBParamToValue( any_type.param(), internalType, ret );
}
    
void PBContainerToComplex( const Container& container, co::IType* descriptor, 
                                          const co::Any& ret )
{
    CORAL_LOG(INFO) << "Starting conversion of a Complex " << descriptor->getName();
    assert( descriptor->getKind() != co::TK_ARRAY );
    
    co::IRecordType* rt = co::cast<co::IRecordType>( descriptor );
    co::IReflector* refl = rt->getReflector();
    
    if( !container.has_complex_type() )
        throw std::string( "No complex type data in parameter" );
    
    const Complex_Type& complex = container.complex_type();
    
    co::TSlice<co::IField*> fields = rt->getFields();
    co::int32 fieldCount = fields.getSize();
    for( co::int32 i = 0; i < fieldCount; i++ )
    {
        co::IField* field = fields[i];
        CORAL_LOG(INFO) << "Field " << field->getName() << " of Type " << field->getType()->getKind();
        const Parameter& fieldArg = complex.field( i );
        co::AnyValue fieldAv; co::Any fieldAny( fieldAv );
        fieldAv.create( field->getType() );
        PBParamToValue( fieldArg, field->getType(), fieldAny );
        if( fieldAny.isValid() )
            refl->setField( ret, field, fieldAny );
    }
}
    
void PBParamToComplex( const Parameter& param, co::IType* descriptor, const co::Any& ret )
{
    if( descriptor->getKind() != co::TK_ARRAY )
    {
        PBContainerToComplex( param.container( 0 ), descriptor, ret );
        return;
    }
    
    co::IType* elementType = co::cast<co::IArray>( descriptor )->getElementType();
    
    co::int32 size = param.container().size();
    
    ret.resize( size );
    CORAL_LOG(INFO) << "Starting conversion of a ComplexArray";
    for( co::int32 i = 0; i < size; i++ )
    {
        co::AnyValue elementAv;
        elementAv.create( elementType );
        PBContainerToComplex( param.container( i ), elementType, elementAv );
        ret[i].put( elementAv );
    }
}
    
void ParameterPuller::pullValue( co::IType* descriptor, const co::Any& ret )
{
    try
    {
        PBParamToValue( _invocation->params( _currentParam ), descriptor, ret );
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
    
    size_t paramSize = param.container().size();
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
    
    const Ref_Type& ref_type = param.container( 0 ).ref_type();
    
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
        case Message::EXCEPTION:
        {
            if( _message->has_exception() )
                _msgType = EXCEPTION;
            
            break;
        }
        case Message::BARRIER_UP:
        {
            _msgType = BARRIER_UP;
            break;
        }
        case Message::BARRIER_HIT:
        {
            _msgType = BARRIER_HIT;
            break;
        }
        case Message::BARRIER_DOWN:
        {
            _msgType = BARRIER_DOWN;
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
    
    if( !request.IsInitialized() || !request.has_instance_type() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    instanceType = request.instance_type();
}

void Demarshaller::getLookup( outString requesterEndpoint, outString lookupKey, outString instanceType )
{
    assert( _msgType == REQUEST_LOOKUP );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Request& request = _message->request();
    
    if( !request.IsInitialized() || !request.has_lookup_key() || !request.has_instance_type() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    lookupKey = request.lookup_key();
    instanceType = request.instance_type();
}
    
void Demarshaller::getLease( outString requesterEndpoint, co::int32& leaseInstanceID )
{
    assert( _msgType == REQUEST_LEASE );
    
    requesterEndpoint = _message->requester_endpoint();
    
    const Request& request = _message->request();
    
    if( !request.IsInitialized() || !request.has_lease_instance_id() )
        CORAL_THROW( RemotingException, "Invalid request data" );
    
    leaseInstanceID = request.lease_instance_id();
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

void Demarshaller::getValueTypeReturn( co::IType* descriptor, const co::Any& ret )
{
    assert( _msgType == RETURN );
    
    const Parameter& returnValue = _message->ret_value();

    try
    {
        PBParamToValue( returnValue, descriptor, ret );
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
    
    size_t paramSize = PBParam.container().size();
    if( paramSize < 1 )
    {
        CORAL_THROW( RemotingException, "Error converting return reference : Empty parameter" );
    }
    else if( paramSize > 1 )
    {
        CORAL_THROW( RemotingException, "Error converting return reference : Multiple references" );
    }
    
    const Ref_Type& ref_type = PBParam.container( 0 ).ref_type();
    
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
    return EX_UNKNOWN;
}
    
void Demarshaller::getBarrierCreator( outString creatorEndpoint )
{
    assert( _msgType == BARRIER_UP );
    
    creatorEndpoint = _message->requester_endpoint();
}
    
} // namespace rpc