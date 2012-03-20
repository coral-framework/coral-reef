#include "MessageUtils.h"

#include "Message.pb.h"

#include <iostream>

namespace reef
{

// ------------- get and set functions specialization for string and bool ----------- //
template <>
const std::string& MessageUtils::getPBContainerData<const std::string&>( const DataContainer& container )
{
    return container.str();
}

template <>
bool MessageUtils::getPBContainerData<bool>( const DataContainer& container )
{
    return container.boolean();
}

template <>
void MessageUtils::setPBContainerData<bool>( DataContainer* container, bool value ) 
{
    container->set_boolean( value );
}

template <>
void MessageUtils::setPBContainerData<const std::string&>( DataContainer* container, const std::string& value ) 
{
    container->set_str( value );
}

// ---------------- anyWithTypeToPBArg specializations for string and bool ------------ //
template <>
void MessageUtils::anyWithTypeToPBArg<std::string>( const co::Any& any, Argument* arg )
{
    // if the Any is a single value, set it directly 
    if( any.getKind() != co::TK_ARRAY )
    {
        DataContainer* container = arg->add_data();
        setPBContainerData<const std::string&>( container, any.get<const std::string&>() );
        return;
    }
    
    // if the Any is an array, iterate through the values adding to the Argument
	const co::Range<const std::string> range = any.get<const co::Range<const std::string> >();
    
    size_t size = range.getSize();
    for( int i = 0; i < size; i++ )
    {
        DataContainer* container = arg->add_data();
        setPBContainerData<const std::string&>( container, range[i] );
    }
}

template <>
void MessageUtils::anyWithTypeToPBArg<bool>( const co::Any& any, Argument* arg )
{
	// if the Any is a single value, set it directly 
	if( any.getKind() != co::TK_ARRAY )
	{
		DataContainer* container = arg->add_data();
		setPBContainerData<bool>( container, any.get<bool>() );
		return;
	}

	// if the Any is an array, iterate through the values adding to the Argument
	const std::vector<bool>& vec = any.get<const std::vector<bool> &>();

	size_t size = vec.size();
	for( int i = 0; i < size; i++ )
	{
		DataContainer* container = arg->add_data();
		setPBContainerData<bool>( container, vec[i] );
	}
}


// ----------------- PBArgWithTypeToAny specializations for string and bool --------------- //
template <>
void MessageUtils::PBArgWithTypeToAny<std::string>( const Argument& arg, co::Any& any, co::IType* elementType )
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

// ------------------- End of template functions specializations --------------- //

void MessageUtils::anyToPBArg( const co::Any& any, Argument* arg )
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

void MessageUtils::PBArgToAny( const Argument& arg, co::IType* descriptor, co::Any& any )
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

void MessageUtils::makeCallMessage( co::int32 destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
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
    
void MessageUtils::makeSetFieldMessage( co::int32 destination, Message& owner, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
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

void MessageUtils::makeGetFieldMessage( co::int32 destination, Message& owner, co::int32 serviceId, co::int32 fieldIndex )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_FIELD );
    
    Message_Member* mf = owner.mutable_msgmember();
    mf->set_serviceindex( serviceId );
    mf->set_hasreturn( true ); // it is a get field event
    mf->set_memberindex( fieldIndex );
}

}