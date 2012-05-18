#include <gtest/gtest.h>

#include <Marshaller.h>
#include <Unmarshaller.h>

#include <co/Coral.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IParameter.h>
#include <co/IInterface.h>
#include <co/ITypeManager.h>


namespace reef {
namespace rpc {


template<typename T> // expects the uint8 array to be already properly sized
static void fillUint8Array( const T value, std::vector<co::uint8>& dest, co::int32 i )
{
    T* toCast = reinterpret_cast<T*>( &dest[0] );
    toCast[i] = value;
}

typedef Unmarshaller::MsgType MsgType;
    
TEST( CodecTests, simpleTypesTest )
{
    Unmarshaller unmarshaller;
    Marshaller marshaller;
    
    // parameters common to message types
    std::string msg;
    std::string referer;
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    co::int32 instanceId;
    
    
    // ------ new inst
    std::string typeName;
    
    marshaller.marshalNewInstance( "test", "address", msg );
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, Unmarshaller::NEW_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_TRUE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address" );
    
    EXPECT_NO_THROW( unmarshaller.unmarshalNewInstance( typeName ) );
    EXPECT_STREQ( typeName.c_str(), "test" );
    
    marshaller.marshalFindInstance( "key", "address2", msg );
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, Unmarshaller::FIND_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_TRUE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address2" );

    std::string key;
    
    EXPECT_NO_THROW( unmarshaller.unmarshalFindInstance( key ) );
    EXPECT_STREQ( key.c_str(), "key" );
   
    // ------ accessinst
    bool increment;
    
    marshaller.marshalAccessInstance( 5, true, "address3", msg );
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, Unmarshaller::ACCESS_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_FALSE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address3" );
    
    EXPECT_NO_THROW( unmarshaller.unmarshalAccessInstance( instanceId, increment ) );
    EXPECT_EQ( instanceId, 5 );
    EXPECT_TRUE( increment );
    
    // ------ call value types TODO:Complex types
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::int32 memberOwner;
    
    // all the possible parameter types
    co::Any intAny; intAny.set<co::int32>( 6 );
    co::Any doubleAny; doubleAny.set<double>( 6.0 );
    co::Any stringParam;
    std::string& stringParamString = stringParam.createString();
    stringParamString = "hello";
    co::Any boolAny; boolAny.set<bool>( true );
    co::Any intArrayAny;
    co::Any stringArrayAny;
    std::vector<co::uint8>& intArray = intArrayAny.createArray( co::getType( "int32" ), 10 );
    std::vector<co::uint8>& stringArray = stringArrayAny.createArray( co::getType( "string" ), 10 );
    for( int i = 0; i < 10; i++ )
	{
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
		fillUint8Array<std::string>( std::string( letter ), stringArray, i );
		fillUint8Array<co::int32>( i, intArray, i );
	}
    
    marshaller.beginCallMarshalling( 3, 4, 5, 6, true );
    marshaller.addValueParam( intAny );
    marshaller.addValueParam( doubleAny );
    marshaller.addValueParam( stringParam );
    marshaller.addValueParam( boolAny );
    marshaller.addValueParam( intArrayAny );
    marshaller.addValueParam( stringArrayAny );
    marshaller.getMarshalledCall( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_EQ( msgType, Unmarshaller::CALL );
    EXPECT_EQ( msgReceiverID, 3 );
    EXPECT_TRUE( hasReturn );
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, memberOwner );
    EXPECT_EQ( facetIdx, 4 );
    EXPECT_EQ( memberIdx, 5 );
    EXPECT_EQ( memberOwner, 6 );
    
    co::IType* intType = co::getType( "int32" );
    co::IType* doubleType = co::getType( "double" );
    co::IType* boolType = co::getType( "bool" );
    co::IType* stringType = co::getType( "string" );
    co::IType* intArrayType = co::getType( "int32[]" );
    co::IType* stringArrayType = co::getType( "string[]" );
    
    
    co::Any param;
    unmarshaller.unmarshalValueParam( param, intType );
    EXPECT_EQ( param.get<co::int32>(), 6 );
    unmarshaller.unmarshalValueParam( param, doubleType );
    EXPECT_EQ( param.get<double>(), 6.0 );
    unmarshaller.unmarshalValueParam( param, stringType );
    EXPECT_STREQ( param.get<const std::string&>().c_str(), "hello" );
    unmarshaller.unmarshalValueParam( param, boolType );
    EXPECT_TRUE( param.get<bool>() );
    co::Any intArrayParam;
    unmarshaller.unmarshalValueParam( intArrayParam, intArrayType );
    co::Any stringArrayParam;
    unmarshaller.unmarshalValueParam( stringArrayParam, stringArrayType );
    
    const co::Range<const co::int32> intRange = intArrayParam.get<const co::Range<const co::int32> >();
    const co::Range<const std::string> stringRange = stringArrayParam.get<const co::Range<const std::string> >();
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        EXPECT_STREQ( stringRange[i].c_str(), letter );
        EXPECT_EQ( intRange[i], i );
    }

	// Test rogue values methods
	co::Any value;
    marshaller.marshalValueType( intAny, msg );
	unmarshaller.unmarshalValue( msg, intType, value );
	EXPECT_EQ( value.get<co::int32>(), 6 );

	marshaller.marshalValueType( doubleAny, msg );
	unmarshaller.unmarshalValue( msg, doubleType, value );
	EXPECT_EQ( value.get<double>(), 6.0 );

	marshaller.marshalValueType( stringParam, msg );
	unmarshaller.unmarshalValue( msg, stringType, value );
	EXPECT_STREQ( value.get<const std::string&>().c_str(), "hello" );

	marshaller.marshalValueType( boolAny, msg );
	unmarshaller.unmarshalValue( msg, boolType, value );
	EXPECT_TRUE( value.get<bool>() );

	co::Any intArrayValue;
	marshaller.marshalValueType( intArrayAny, msg );
	unmarshaller.unmarshalValue( msg, intArrayType, intArrayValue );
	co::Any stringArrayValue;
	marshaller.marshalValueType( stringArrayAny, msg );
	unmarshaller.unmarshalValue( msg, stringArrayType, stringArrayValue );

	const co::Range<const co::int32> intRange2 = intArrayValue.get<const co::Range<const co::int32> >();
    const co::Range<const std::string> stringRange2 = stringArrayValue.get<const co::Range<const std::string> >();
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        EXPECT_STREQ( stringRange2[i].c_str(), letter );
        EXPECT_EQ( intRange2[i], i );
    }
}
    
}
    
}