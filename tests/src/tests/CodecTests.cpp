/*#include <gtest/gtest.h>

#include <Message.pb.h>
#include <MessageUtils.h>

#include <co/Coral.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IParameter.h>
#include <co/IInterface.h>


namespace reef
{

template<typename T> // expects the uint8 array to be already properly sized
static void fillUint8Array( const T value, std::vector<co::uint8>& dest, co::int32 i )
{
	T* toCast = reinterpret_cast<T*>( &dest[0] );
	toCast[i] = value;
}

template <typename T>
static bool compareArraysOfSameType( const co::Any& any1, const co::Any& any2 )
{
	const co::Range<const T> range1 = any1.get<const co::Range<const T> >();
	const co::Range<const T> range2 = any2.get<const co::Range<const T> >();

	for( int i = 0; i < range1.getSize(); i++ )
	{
		if( range1[i] != range2[i] )
			return false;
	}
	return true;
}

static bool compareAnys( const co::Any& any1, const co::Any& any2 )
{
	if( any1.getKind() != any2.getKind() )
		return false;

	co::TypeKind kind = any1.getKind();
	if( kind == co::TK_ARRAY )
	{
		bool result = false;

		kind = any1.getType()->getKind();
		switch( kind )
		{
		case co::TK_INT32:
			result = compareArraysOfSameType<co::int32>( any1, any2 );
			break;
		case co::TK_DOUBLE:
			result = compareArraysOfSameType<double>( any1, any2 );
			break;
		case co::TK_STRING:
			result = compareArraysOfSameType<std::string>( any1, any2 );
			break;
		default:
			assert( false );
		}
		return result;
	}
	else if( kind == co::TK_STRING )
	{
		return any1.get<const std::string&>() == any2.get<const std::string&>();
	}
	else
	{
		return any1 == any2;
	}
}

// check if the message is correct
static bool checkCallMessage( const Message* msg, Message::Type typ, co::int32 destId, 
	co::int32 servId, co::int32 membId, co::IMethod* method, co::Range<co::Any const> args )
{
	bool result = true;
	result = result && ( msg->type() == typ );
	result = result && ( msg->destination() == destId );

	const Message_Member& member = msg->msgmember();
	result = result && ( member.serviceindex() == servId );
	result = result && ( member.memberindex() == membId );

	co::Range<co::IParameter* const> params = method->getParameters();

	
	size_t size = member.arguments().size();
	for( int i = 0; i < size; i++ )
	{
		co::Any any;
		MessageUtils::PBArgToAny( member.arguments( i ), params[i]->getType(), any );
		
		result = result && compareAnys( args[i], any );
	}

	return result;
}

// check if the message is correct
static bool checkFieldMessage( const Message* msg, Message::Type typ, co::int32 destId, 
	co::int32 servId, co::int32 membId, co::IField* field, co::Any value )
{
	bool result = true;
	result = result && ( msg->type() == typ );
	result = result && ( msg->destination() == destId );

	const Message_Member& member = msg->msgmember();
	result = result && ( member.serviceindex() == servId );
	result = result && ( member.memberindex() == membId );

	co::Any any;
	MessageUtils::PBArgToAny( member.arguments( 0 ), field->getType(), any );
		
	result = result && compareAnys( value, any );

	return result;
}

TEST( CodecTests, simpleTypesTest )
{
	// message variable format: type_dest_service_member_params
	Message call_5_3_4_sArr_sArr;
	Message call_10_25_999_dArr_dArr;
	Message sendcall_3_7_9_s;
	Message setfield_2_3_4_iArr;

	// create the parameters for the messages
	std::vector<co::Any> s_arg;
	std::vector<co::Any> iArr_arg;
	std::vector<co::Any> sArr_sArr_arg;
	std::vector<co::Any> dArr_dArr_arg;
	s_arg.resize( 1 ); iArr_arg.resize( 1 ); sArr_sArr_arg.resize( 2 ); dArr_dArr_arg.resize( 2 );

	// fill the parameters
	// string param
	std::string temp( "hello" );
	s_arg[0].set<const std::string&>( temp );

	// int32[] param
	std::vector<co::uint8>& iArr = iArr_arg[0].createArray( co::getType( "int32" ), 10 );

	// string[], string[] params
	std::vector<co::uint8>& strArr1 = sArr_sArr_arg[0].createArray( co::getType( "string" ), 10 );
	std::vector<co::uint8>& strArr2 = sArr_sArr_arg[1].createArray( co::getType( "string" ), 10 );
	
	// double[], double[] params
	std::vector<co::uint8>& dArr1 = dArr_dArr_arg[0].createArray( co::getType( "double" ), 10 );
	std::vector<co::uint8>& dArr2 = dArr_dArr_arg[1].createArray( co::getType( "double" ), 10 );
	
	
	for( int i = 0; i < 10; i++ )
	{
		char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
		fillUint8Array<std::string>( std::string( letter ), strArr1, i );
		fillUint8Array<std::string>( std::string( letter ), strArr2, i );
		fillUint8Array<co::int32>( i, iArr, i );
		fillUint8Array<double>( static_cast<double>( i ), dArr1, i );
		fillUint8Array<double>( static_cast<double>( i ), dArr2, i );
	}

	co::IInterface* STInterface = co::cast<co::IInterface>( co::getType( "testModule.ISimpleTypes" ) );
	co::IMethod* getThirdElements = co::cast<co::IMethod>( STInterface->getMember( "getThirdElements" ) );
	co::IMethod* mergeMethod = co::cast<co::IMethod>( STInterface->getMember( "mergeLists" ) );
	co::IMethod* setStringMethod = co::cast<co::IMethod>( STInterface->getMember( "setString" ) );
	co::IField* storedIntListField = co::cast<co::IField>( STInterface->getMember( "storedIntList" ) );

	// Create the messages
	MessageUtils::makeCallMessage( 5, true, call_5_3_4_sArr_sArr, 3, 4, sArr_sArr_arg );
	MessageUtils::makeCallMessage( 10, true, call_10_25_999_dArr_dArr, 25, 999, dArr_dArr_arg );
	MessageUtils::makeCallMessage( 3, true, sendcall_3_7_9_s, 7, 9, s_arg );
	MessageUtils::makeSetFieldMessage( 2, setfield_2_3_4_iArr, 3, 4, iArr_arg[0] );
	
	EXPECT_TRUE( checkCallMessage( &call_5_3_4_sArr_sArr, Message::TYPE_CALL, 5, 3, 4, getThirdElements, 
		sArr_sArr_arg ) );
	EXPECT_TRUE( checkCallMessage( &call_10_25_999_dArr_dArr, Message::TYPE_CALL, 10, 25, 999, mergeMethod,
		dArr_dArr_arg ) );
	EXPECT_TRUE( checkCallMessage( &sendcall_3_7_9_s, Message::TYPE_CALL, 3, 7, 9, setStringMethod, s_arg ) );
	EXPECT_TRUE( checkFieldMessage( &setfield_2_3_4_iArr, Message::TYPE_FIELD, 2,3,4, storedIntListField, iArr_arg[0] ) );
}

}*/