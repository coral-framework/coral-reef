#include <gtest/gtest.h>

#include <Marshaller.h>
#include <Demarshaller.h>

#include <co/Coral.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IParameter.h>
#include <co/IInterface.h>
#include <co/ITypeManager.h>

#define INT1 10
#define INT2 20
#define INT3 30
#define INT4 40
#define BOOL1 true
#define DOUBLE1 10.0
#define STRING1 "string1"
#define STRING2 "string2"
#define STRING3 "string3"

namespace rpc {


template<typename T> // expects the uint8 array to be already properly sized
static void fillUint8Array( const T value, std::vector<co::uint8>& dest, co::int32 i )
{
    T* toCast = reinterpret_cast<T*>( &dest[0] );
    toCast[i] = value;
}

bool compareDetails( InvocationDetails& det1, InvocationDetails& det2 )
{
    bool result = true;
    result = result && det1.instanceID == det2.instanceID;
    result = result && det1.facetIdx == det2.facetIdx;
    result = result && det1.memberIdx == det2.memberIdx;
    result = result && det1.typeDepth == det2.typeDepth;
    result = result && det1.hasReturn == det2.hasReturn;
    return result;
}
    
TEST( CodecTests, simpleTypesTest )
{
    Demarshaller demarshaller;
    Marshaller marshaller;
    
    // parameters common to message types
    std::string msg;
    std::string requester;
    MessageType msgType;
    
    std::string instanceType;
    marshaller.marshalNew( STRING1, STRING2, msg );
    msgType = demarshaller.demarshal( msg );
    demarshaller.getNew( requester, instanceType );
    
    EXPECT_EQ( msgType, REQUEST_NEW );
    EXPECT_STREQ( requester.c_str(), STRING1 );
    EXPECT_STREQ( instanceType.c_str(), STRING2 );
    
    std::string key;
    marshaller.marshalLookup( STRING1, STRING2, STRING3, msg );
    msgType = demarshaller.demarshal( msg );
    demarshaller.getLookup( requester, key, instanceType );
    
    EXPECT_EQ( msgType, REQUEST_LOOKUP );
    EXPECT_STREQ( requester.c_str(), STRING1 );
    EXPECT_STREQ( key.c_str(), STRING2 );
    EXPECT_STREQ( instanceType.c_str(), STRING3 );

    co::int32 leaseInstanceID;
    marshaller.marshalLease( STRING1, INT1, msg );
    msgType = demarshaller.demarshal( msg );
    demarshaller.getLease( requester, leaseInstanceID );
    
    EXPECT_EQ( msgType, REQUEST_LEASE );
    EXPECT_STREQ( requester.c_str(), STRING1 );
    EXPECT_EQ( leaseInstanceID, INT1 );
    
    // ------ call value types TODO:Complex types
    
    // all the possible parameter types
    co::Any intAny; intAny.set<co::int32>( INT1 );
    co::Any doubleAny; doubleAny.set<double>( DOUBLE1 );
    co::Any stringParam;
    std::string& stringParamString = stringParam.createString();
    stringParamString = STRING1;
    co::Any boolAny; boolAny.set<bool>( BOOL1 );
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
    
    InvocationDetails details( INT1, INT2, INT3, INT4, BOOL1 );
    ParameterPusher& pusher = marshaller.beginInvocation( STRING1, details );
    pusher.pushValue( intAny );
    pusher.pushValue( doubleAny );
    pusher.pushValue( stringParam );
    pusher.pushValue( boolAny );
    pusher.pushValue( intArrayAny );
    pusher.pushValue( stringArrayAny );
    marshaller.marshalInvocation( msg );
    
    msgType = demarshaller.demarshal( msg );
    
    EXPECT_EQ( msgType, INVOCATION );
    
    InvocationDetails details2;
    ParameterPuller& puller = demarshaller.getInvocation( requester, details2 );
    EXPECT_TRUE( compareDetails( details, details2 ) );
    
    co::IType* intType = co::getType( "int32" );
    co::IType* doubleType = co::getType( "double" );
    co::IType* boolType = co::getType( "bool" );
    co::IType* stringType = co::getType( "string" );
    co::IType* intArrayType = co::getType( "int32[]" );
    co::IType* stringArrayType = co::getType( "string[]" );
    
    
    co::Any param;
    
    puller.pullValue( intType, param );
    EXPECT_EQ( param.get<co::int32>(), INT1 );
    
    puller.pullValue( doubleType, param );
    EXPECT_EQ( param.get<double>(), DOUBLE1 );
    
    puller.pullValue( stringType, param );
    EXPECT_STREQ( param.get<const std::string&>().c_str(), STRING1 );
    
    puller.pullValue( boolType, param );
    EXPECT_EQ( param.get<bool>(), BOOL1 );
    
    co::Any intArrayParam;
    puller.pullValue( intArrayType, intArrayParam );
    
    co::Any stringArrayParam;
    puller.pullValue( stringArrayType, stringArrayParam );
    
    co::TSlice<co::int32> intRange = intArrayParam.get<co::TSlice<co::int32> >();
    co::TSlice<std::string> stringRange = stringArrayParam.get<co::TSlice<std::string> >();
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
    marshaller.marshalValueTypeReturn( intAny, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( intType, value );
	EXPECT_EQ( value.get<co::int32>(), INT1 );

	marshaller.marshalValueTypeReturn( doubleAny, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( doubleType, value );
	EXPECT_EQ( value.get<double>(), DOUBLE1 );

	marshaller.marshalValueTypeReturn( stringParam, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( stringType, value );
	EXPECT_STREQ( value.get<const std::string&>().c_str(), STRING1 );

	marshaller.marshalValueTypeReturn( boolAny, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( boolType, value );
	EXPECT_EQ( param.get<bool>(), BOOL1 );

	co::Any intArrayValue;
	marshaller.marshalValueTypeReturn( intArrayAny, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( intArrayType, intArrayValue );
    
	co::Any stringArrayValue;
	marshaller.marshalValueTypeReturn( stringArrayAny, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( stringArrayType, stringArrayValue );

	co::TSlice<co::int32> intRange2 = intArrayValue.get<co::TSlice<co::int32> >();
    co::TSlice<std::string> stringRange2 = stringArrayValue.get<co::TSlice<std::string> >();
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        EXPECT_STREQ( stringRange2[i].c_str(), letter );
        EXPECT_EQ( intRange2[i], i );
    }
}
    
} // namespace rpc