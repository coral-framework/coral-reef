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

	co::IType* intType = co::getType( "int32" );
    co::IType* doubleType = co::getType( "double" );
    co::IType* boolType = co::getType( "bool" );
    co::IType* stringType = co::getType( "string" );
    co::IType* intArrayType = co::getType( "int32[]" );
    co::IType* stringArrayType = co::getType( "string[]" );

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
    co::AnyValue intAny( INT1 );
    co::AnyValue doubleAny( DOUBLE1 );
    
	std::string stringRef( STRING1 );

	co::Any stringParam( stringRef );
	co::AnyValue boolAny( BOOL1 );  

    std::vector<co::int32> intArray;
	std::vector<std::string> stringArray;
	
    for( int i = 0; i < 10; i++ )
	{
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
		
		intArray.push_back( i );
		stringArray.push_back( std::string( letter ) );

	}
    
	co::Any intArrayAny = intArray;
	
	co::Any stringArrayAny = stringArray;

    InvocationDetails details( INT1, INT2, INT3, INT4, BOOL1 );
    ParameterPusher& pusher = marshaller.beginInvocation( STRING1, details );

    pusher.pushValue( intAny, intType );
	pusher.pushValue( doubleAny, doubleType );
    pusher.pushValue( stringParam, stringType );
    pusher.pushValue( boolAny, boolType );
    pusher.pushValue( intArrayAny, intArrayType );
    pusher.pushValue( stringArrayAny, stringArrayType );
    marshaller.marshalInvocation( msg );
    
    msgType = demarshaller.demarshal( msg );
    
    EXPECT_EQ( msgType, INVOCATION );
    
    InvocationDetails details2;
    ParameterPuller& puller = demarshaller.getInvocation( requester, details2 );
    EXPECT_TRUE( compareDetails( details, details2 ) );
    
    co::AnyValue param;
    
    puller.pullValue( intType, param );
    EXPECT_EQ( param.get<co::int32>(), INT1 );
    
    puller.pullValue( doubleType, param );
    EXPECT_EQ( param.get<double>(), DOUBLE1 );
    
    puller.pullValue( stringType, param );
    EXPECT_STREQ( param.get<const std::string&>().c_str(), STRING1 );
    
    puller.pullValue( boolType, param );
    EXPECT_EQ( param.get<bool>(), BOOL1 );
    
    co::AnyValue intArrayParam;
	intArrayParam.create( intArrayType );
	puller.pullValue( intArrayType, intArrayParam.getAny() );
    
    co::AnyValue stringArrayParam;
	stringArrayParam.create( stringArrayType );
	puller.pullValue( stringArrayType, stringArrayParam.getAny() );
    
	std::vector<std::string> stringArrayResult = stringArrayParam.get<std::vector<std::string>>();
	std::vector<co::int32> intArrayResult = intArrayParam.get<std::vector<co::int32>>();

    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
		EXPECT_STREQ( stringArrayResult[i].c_str(), letter );
        EXPECT_EQ( intArrayResult[i], i );
    }

	// Test rogue values methods
	co::AnyValue value;
    marshaller.marshalValueTypeReturn( intAny, intType, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( intType, value );
	EXPECT_EQ( value.get<co::int32>(), INT1 );

	marshaller.marshalValueTypeReturn( doubleAny, doubleType, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( doubleType, value );
	EXPECT_EQ( value.get<double>(), DOUBLE1 );

	marshaller.marshalValueTypeReturn( stringParam, stringType, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( stringType, value );
	EXPECT_STREQ( value.get<const std::string&>().c_str(), STRING1 );

	marshaller.marshalValueTypeReturn( boolAny, boolType, msg );
	demarshaller.demarshal( msg );
    demarshaller.getValueTypeReturn( boolType, value );
	EXPECT_EQ( param.get<bool>(), BOOL1 );

	co::AnyValue stringArrayValue;
	stringArrayValue.create( stringArrayType );
	marshaller.marshalValueTypeReturn( stringArrayAny, stringArrayType, msg );
	demarshaller.demarshal( msg );
	demarshaller.getValueTypeReturn( stringArrayType, stringArrayValue.getAny() );

	co::AnyValue intArrayValue;
	intArrayValue.create( intArrayType );
	marshaller.marshalValueTypeReturn( intArrayAny, intArrayType, msg );
	demarshaller.demarshal( msg );
	demarshaller.getValueTypeReturn( intArrayType, intArrayValue.getAny() );

	stringArrayResult = stringArrayValue.get<std::vector<std::string>>();
	intArrayResult = intArrayValue.get<std::vector<co::int32>>();

    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        EXPECT_STREQ( stringArrayResult[i].c_str(), letter );
        EXPECT_EQ( intArrayResult[i], i );
    }
}
    
} // namespace rpc