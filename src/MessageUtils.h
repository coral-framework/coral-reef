#ifndef _REEF_MESSAGEBUILDER_H_
#define _REEF_MESSAGEBUILDER_H_

#include <co/Range.h>
#include <co/Any.h>
#include <co/TypeKind.h>
#include <co/IParameter.h>
#include "Message.pb.h"

namespace reef
{

class MessageUtils
{
public:
	// Converts an Any(Coral) to an Argument(Protobuf/reef). Numbers, strings and arrays supported.
	static void MessageUtils::anyToPBArg( const co::Any& any, Argument* arg );

	/* Converts an Argument(Protobuf/reef) to an Any(Coral) that will be used as a parameter. 
	\descriptor is the actual parameter that the Any will represent. Numbers, strings and arrays supported.
	*/
	static void PBArgToAny( const Argument& arg, co::IParameter* descriptor, co::Any& any );

	static Message_Call* MessageUtils::makeCallMessage( int destination, bool hasReturn, Message& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args );

	static Message_Field* MessageUtils::makeFieldMessage( int destination, bool isSet, Message& owner, co::int32 serviceId, co::int32 fieldIndex );

private:

	// ------------ Any to Protobuf conversion functions ----------------- //
	// Extracts the provided type's data from Any (deals with arrays and values)
	template <typename T>
	static void anyWithTypeToPBArg( const co::Any& any, Argument* arg )
	{
		// if the Any is a single value, set it directly 
		if( any.getKind() != co::TK_ARRAY )
		{
			DataContainer* container = arg->add_data();
			setPBContainerData<T>( container, any.get<T>() );
			return;
		}

		// if the Any is an array, iterate through the values adding to the Argument
		std::vector<T>& vec = any.get<std::vector<T> &>();

		size_t size = vec.size();
		for( int i = 0; i < size; i++ )
		{
			DataContainer* container = arg->add_data();
			setPBContainerData<T>( container, vec[i] );
		}
	}
	// Specialization for string
	template <>
	static void anyWithTypeToPBArg<std::string>( const co::Any& any, Argument* arg )
	{
		// if the Any is a single value, set it directly 
		if( any.getKind() != co::TK_ARRAY )
		{
			DataContainer* container = arg->add_data();
			setPBContainerData<const std::string&>( container, any.get<const std::string&>() );
			return;
		}

		// if the Any is an array, iterate through the values adding to the Argument
		std::vector<const std::string>& vec = any.get<std::vector<const std::string> &>();

		size_t size = vec.size();
		for( int i = 0; i < size; i++ )
		{
			DataContainer* container = arg->add_data();
			setPBContainerData<const std::string&>( container, vec[i] );
		}
	}

	// Specializes for each Data container's different set function.
	template <typename T>
	inline static void setPBContainerData( DataContainer* container, T value ) 
	{
		container->set_numeric( value );
	}
	template <>
	inline static void setPBContainerData<bool>( DataContainer* container, bool value ) 
	{
		container->set_boolean( value );
	}
	template <>
	inline static void setPBContainerData<const std::string&>( DataContainer* container, const std::string& value ) 
	{
		container->set_str( value );
	}


	// -------------- Protobuf to Any conversion functions ------------------//
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
		std::vector<co::uint8>& vec = any.createArray( elementType, size );
		T* toCast = reinterpret_cast<T*>( &vec[0] );
		for( int i = 0; i < size; i++ )
		{
			toCast[i] = getPBContainerData<T>( arg.data( i ) );
		}
	}

	template <>
	static void PBArgWithTypeToAny<std::string>( const Argument& arg, co::Any& any, co::IType* elementType )
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

	// Specializes for each Data container's different get function.
	template <typename T>
	static T getPBContainerData( const DataContainer& container )
	{
		return static_cast<T>( container.numeric() );
	}
	template <>
	static bool getPBContainerData<bool>( const DataContainer& container )
	{
		return container.boolean();
	}
	template <>
	static const std::string& getPBContainerData<const std::string&>( const DataContainer& container )
	{
		return container.str();
	}
};

}

#endif