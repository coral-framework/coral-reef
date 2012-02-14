#include "MessageUtils.h"

#include "Message.pb.h"

#include <iostream>

namespace reef
{
    
// Specialization for string
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
    const std::vector<std::string>& vec = any.get<const std::vector<std::string> &>();
    
    size_t size = vec.size();
    for( int i = 0; i < size; i++ )
    {
        DataContainer* container = arg->add_data();
        setPBContainerData<const std::string&>( container, vec[i] );
    }
}

template <>
void MessageUtils::PBArgWithTypeToAny<std::string>( const Argument& arg, co::Any& any, co::IType* elementType )
{
    if( !elementType )
    {
        any.set<const std::string&>( getPBContainerData<const std::string&>( arg.data( 0 ) ) );
        return;
    }
    
    size_t size = arg.data().size();
    std::vector<co::uint8>& vec = any.createArray( elementType, size );
    std::string* toCast = reinterpret_cast<std::string*>( &vec[0] );
    for( int i = 0; i < size; i++ )
    {
        toCast[i] = getPBContainerData<const std::string&>( arg.data( i ) );
    }
}

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
	case co::TK_UINT8:
	case co::TK_INT16:
	case co::TK_UINT16:
	case co::TK_INT32:
	case co::TK_UINT32:
	case co::TK_INT64:
	case co::TK_UINT64:
	case co::TK_FLOAT:
	case co::TK_DOUBLE:
		anyWithTypeToPBArg<double>( any, arg );
		break;
	case co::TK_STRING:
		anyWithTypeToPBArg<std::string>( any, arg );
	default:
		assert( false );
	}
}

void MessageUtils::PBArgToAny( const Argument& arg, co::IParameter* descriptor, co::Any& any )
{
	co::IType* typ = descriptor->getType();
	co::TypeKind kind = typ->getKind();
	co::IType* elementType = 0; // only used for arrays
	
	if( kind == co::TK_ARRAY )
	{
		elementType = co::cast<co::IArray>( typ )->getElementType();
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

Message_Call* MessageUtils::makeCallMessage( int destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    owner.set_destination( destination );
    owner.set_type( Message::TYPE_CALL );
	
    Message_Call* mc = owner.mutable_msgcall();
    mc->set_hasreturn( hasReturn );
    mc->set_serviceindex( serviceId );
    mc->set_memberindex( methodIndex );
    for( ; args; args.popFirst() )
	{
		Argument* PBArg = mc->add_arguments();
		MessageUtils::anyToPBArg( args.getFirst(), PBArg );
	}
    
    return mc;
}
    
Message_Field* MessageUtils::makeFieldMessage( int destination, bool isSet, Message& owner, co::int32 serviceId, co::int32 fieldIndex )
{
    owner.set_destination( destination ); // 0 is always the node channel
    owner.set_type( Message::TYPE_FIELD );
    
    Message_Field* mf = owner.mutable_msgfield();
    mf->set_issetfield( isSet ); // it is a get field event
    mf->set_serviceindex( serviceId );
    mf->set_fieldindex( fieldIndex );

    return mf;
}

}