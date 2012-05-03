#include <gtest/gtest.h>

#include <Encoder.h>
#include <Decoder.h>

#include <co/Coral.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IParameter.h>
#include <co/IInterface.h>
#include <co/ITypeManager.h>


namespace reef
{

template<typename T> // expects the uint8 array to be already properly sized
static void fillUint8Array( const T value, std::vector<co::uint8>& dest, co::int32 i )
{
    T* toCast = reinterpret_cast<T*>( &dest[0] );
    toCast[i] = value;
}

typedef Decoder::MsgType MsgType;
    
TEST( CodecTests, simpleTypesTest )
{
    Decoder decoder;
    Encoder encoder;
    
    // parameters common to message types
    std::string msg;
    std::string referer;
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    co::int32 instanceID;
    
    
    // ------ new inst
    std::string typeName;
    
    encoder.encodeNewInstMsg( "test", msg, "address" );
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, MsgType::NEW_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_TRUE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address" );
    
    EXPECT_NO_THROW( decoder.decodeNewInstMsg( typeName ) );
    EXPECT_STREQ( typeName.c_str(), "test" );
    
    encoder.encodeFindInstMsg( "key", msg, "address2" );
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, MsgType::FIND_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_TRUE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address2" );

    std::string key;
    
    EXPECT_NO_THROW( decoder.decodeFindInstMsg( key ) );
    EXPECT_STREQ( key.c_str(), "key" );
   
    // ------ accessinst
    bool increment;
    
    encoder.encodeAccessInstMsg( 5, true, msg, "address3" );
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn, &referer );
    
    EXPECT_EQ( msgType, MsgType::ACCESS_INST );
    EXPECT_EQ( msgReceiverID, 0 );
    EXPECT_FALSE( hasReturn );
    EXPECT_STREQ( referer.c_str(), "address3" );
    
    EXPECT_NO_THROW( decoder.decodeAccessInstMsg( instanceID, increment ) );
    EXPECT_EQ( instanceID, 5 );
    EXPECT_TRUE( increment );
    
    // ------ call value types TODO:Complex types
    co::int32 facetIdx;
    co::int32 memberIdx;
    
    // all the possible parameter types
    co::Any intParam; intParam.set<co::int32>( 6 );
    co::Any doubleParam; doubleParam.set<double>( 6.0 );
    co::Any stringParam;
    std::string& stringParamString = stringParam.createString();
    stringParamString = "hello";
    co::Any boolParam; boolParam.set<bool>( true );
    co::Any intArrayParam;
    co::Any stringArrayParam;
    std::vector<co::uint8>& intArray = intArrayParam.createArray( co::getType( "int32" ), 10 );
    std::vector<co::uint8>& stringArray = stringArrayParam.createArray( co::getType( "string" ), 10 );
    for( int i = 0; i < 10; i++ )
	{
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
		fillUint8Array<std::string>( std::string( letter ), stringArray, i );
		fillUint8Array<co::int32>( i, intArray, i );
	}
    
    encoder.beginEncodingCallMsg( 3, 4, 5, true );
    encoder.addValueParam( intParam );
    encoder.addValueParam( doubleParam );
    encoder.addValueParam( stringParam );
    encoder.addValueParam( boolParam );
    encoder.addValueParam( intArrayParam );
    encoder.addValueParam( stringArrayParam );
    encoder.finishEncodingCallMsg( msg );
    
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_EQ( msgType, MsgType::CALL );
    EXPECT_EQ( msgReceiverID, 3 );
    EXPECT_TRUE( hasReturn );
    
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, 4 );
    EXPECT_EQ( memberIdx, 5 );
    
    co::IType* intType = co::getType( "int32" );
    co::IType* doubleType = co::getType( "double" );
    co::IType* boolType = co::getType( "bool" );
    co::IType* stringType = co::getType( "string" );
    co::IType* intArrayType = co::getType( "int32[]" );
    co::IType* stringArrayType = co::getType( "string[]" );
    
    
    co::Any param;
    decoder.getValueParam( param, intType );
    EXPECT_EQ( param.get<co::int32>(), 6 );
    decoder.getValueParam( param, doubleType );
    EXPECT_EQ( param.get<double>(), 6.0 );
    decoder.getValueParam( param, stringType );
    EXPECT_STREQ( param.get<const std::string&>().c_str(), "hello" );
    decoder.getValueParam( param, boolType );
    EXPECT_TRUE( param.get<bool>() );
    co::Any intArrayParam2;
    decoder.getValueParam( intArrayParam2, intArrayType );
    co::Any stringArrayParam2;
    decoder.getValueParam( stringArrayParam2, stringArrayType );
    
    const co::Range<const co::int32> intRange = intArrayParam2.get<const co::Range<const co::int32> >();
    const co::Range<const std::string> stringRange = stringArrayParam2.get<const co::Range<const std::string> >();
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        EXPECT_STREQ( stringRange[i].c_str(), letter );
        EXPECT_EQ( intRange[i], i );
    }

}

}