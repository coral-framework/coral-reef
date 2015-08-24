#include <gtest/gtest.h>

#include <stubs/ChildStruct.h>
#include <stubs/MotherStruct.h>
#include <stubs/ISimpleTypes.h>
#include <stubs/IComplexTypes.h>
#include <stubs/IReferenceTypes.h>
#include <stubs/StringNativeClass.h>
#include <stubs/ITestSetup.h>
#include <stubs/DummyEnum.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>

#include <co/Log.h>
#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#define INT1 10
#define INT2 20
#define INT3 30
#define INT4 40
#define BOOL1 true
#define DOUBLE1 10.0
#define STRING1 "string1"
#define STRING2 "string2"
#define TESTVECSIZE 2
#define CHAR1 65
#define CHAR2 75
#define CHAR3 85

namespace rpc {

    
TEST( ParameterTests, simpleTypesTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* client = setup->getNode( 1 );
	rpc::INode* server = setup->getNode( 2 );

	co::IObjectRef originalObject = co::newInstance( "stubs.TestComponent" );
    
	server->publishInstance( originalObject.get(), "remoteInstance" );

    co::RefPtr<co::IObject> remoteInstance = client->getRemoteInstance( "stubs.TestComponent", "remoteInstance", "address2" );
    stubs::ISimpleTypes* simple = remoteInstance->getService<stubs::ISimpleTypes>();
    
    // ------ Simple value types ------
    simple->setDouble( 0.1 );
    EXPECT_EQ( simple->getStoredDouble(), 0.1 );
    
    simple->setStoredInt( 99 );
    EXPECT_EQ( simple->getStoredInt(), 99 );
    
    EXPECT_EQ( simple->divide( 10, 5), 2.0 );
    
    const std::string testString( "1234" );
    simple->setString( testString );
    EXPECT_STREQ( testString.c_str() , simple->getStoredString().c_str() );
    
    // ------ Simple values stored inside Any ------ //
    co::Any d1( 4.0 );
    co::AnyValue dResult = simple->addDoublesFromAny( d1, d1 ); // Cannot pass different anys now
    EXPECT_DOUBLE_EQ( 8, dResult.get<double>() );
    
    std::string str1( STRING1 );
    co::Any str1Any( str1 );
    co::AnyValue strResult( simple->concatenateFromAny( str1Any, str1Any ) );
    EXPECT_STREQ( "string1string1", strResult.get<const std::string&>().c_str() );
    
	EXPECT_EQ( simple->getNextEnum( stubs::one ), stubs::two );

	std::vector<stubs::DummyEnum> enumVec;
	enumVec.push_back( stubs::one );
	enumVec.push_back( stubs::two );
	enumVec.push_back( stubs::three );
	co::TSlice<stubs::DummyEnum> enumSlice = simple->removeLastEnum( enumVec );
	EXPECT_EQ( enumSlice.getSize(), 2 );
	EXPECT_EQ( enumSlice[0], stubs::one );
	EXPECT_EQ( enumSlice[1], stubs::two );

    // ------ Simple value Types Arrays ------ //
    std::vector<co::int32> intVec;
    std::vector<double> doubleVec;
    std::vector<std::string> stringVec;
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        stringVec.push_back( letter );
        intVec.push_back( i );
        doubleVec.push_back( static_cast<double>( i ) );
    }
    
    simple->setStoredIntList( intVec );
    simple->setParentStringList( stringVec );
    
    co::TSlice<co::int32> intRange = simple->getStoredIntList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( intVec[i], intRange[i] );
    
    co::TSlice<double> doubleRange = simple->parentMergeLists( doubleVec, doubleVec );
    int size = doubleRange.getSize();

    for( int i = 0; i < size; i++ )
    {
        double comparison = i % 10;
        double value = doubleRange[i];
        EXPECT_DOUBLE_EQ( comparison, value );
    }
    
    co::TSlice<std::string> stringRange = simple->getParentStringList();
    for( int i = 0; i < intVec.size(); i++ )
       EXPECT_STREQ( stringVec[i].c_str(), stringRange[i].c_str() );
    
    
    EXPECT_EQ( simple->incrementInt( 1 ), 2 );
    EXPECT_DOUBLE_EQ( simple->divide( 10, 5 ), 2 );
    EXPECT_STREQ( "abcdef", simple->concatenateString( "abc", "def" ).c_str() );
    
    std::vector<std::string> stringVec2;
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 21 + i;
		letter[1] = '\0';
        stringVec2.push_back( letter );
    }
    co::TSlice<std::string> stringRange2 = simple->getThirdElements( stringVec, stringVec2 );
    
    EXPECT_STREQ( stringRange2[0].c_str(), stringVec[2].c_str() );
    EXPECT_STREQ( stringRange2[1].c_str(), stringVec2[2].c_str() );
    
	co::int32 two; co::int32 three; co::int32 four; co::int32 five;
	co::int32 one = simple->get12345( two, three, four, five );
	EXPECT_EQ( 1, one );
	EXPECT_EQ( 2, two );
	EXPECT_EQ( 3, three );
	EXPECT_EQ( 4, four );
	EXPECT_EQ( 5, five );

    setup->tearDown();
}

TEST( ParameterTests, refTypeParameterTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 3 );
    
	rpc::INode* serverA = setup->getNode( 1 );
    rpc::INode* serverB = setup->getNode( 2 );
    rpc::INode* client = setup->getNode( 3 );

	co::IObjectRef instanceInAOrig = co::newInstance( "stubs.TestComponent" );
	serverA->publishInstance(  instanceInAOrig.get(), "instanceInA" );

	co::RefPtr<co::IObject> instanceInA = client->getRemoteInstance("stubs.TestComponent", "instanceInA", "address1");
    stubs::IReferenceTypes* refTypesServiceInA = instanceInA->getService<stubs::IReferenceTypes>();
    stubs::ISimpleTypes* simpleTypesServiceInA = instanceInA->getService<stubs::ISimpleTypes>();
    
	co::IObjectRef instanceInBOrig = co::newInstance( "stubs.TestComponent" );
	serverB->publishInstance(  instanceInBOrig.get(), "instanceInB" );
	co::RefPtr<co::IObject> instanceInB = client->getRemoteInstance("stubs.TestComponent", "instanceInB", "address2");

    stubs::ISimpleTypes* simpleTypesServiceInB = instanceInB->getService<stubs::ISimpleTypes>();

    
    co::RefPtr<co::IObject> instanceLocal = co::newInstance( "stubs.TestComponent" );
    stubs::ISimpleTypes* simpleTypesServiceLocal = instanceLocal->getService<stubs::ISimpleTypes>();
    
    EXPECT_EQ( refTypesServiceInA->callIncrementInt( simpleTypesServiceInA, 3 ), 4 );
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceInA, 15, 5 ), 3 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceInA, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceLocal, 15, 5 ), 3 );
    EXPECT_EQ( refTypesServiceInA->parentCall( simpleTypesServiceLocal, 3 ), 4 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceLocal, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    
    EXPECT_EQ( refTypesServiceInA->callIncrementInt( simpleTypesServiceInB, 3 ), 4 );
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceInB, 15, 5 ), 3 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceInB, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    refTypesServiceInA->setSimple( simpleTypesServiceInB );
	stubs::ISimpleTypesRef simpleRef;
	refTypesServiceInA->getSimple( simpleRef );
    EXPECT_EQ( simpleTypesServiceInB, simpleRef.get() );
    
    stubs::ISimpleTypes* simple = serverB->getInstance( 0 )->getService<stubs::ISimpleTypes>();
    
    EXPECT_NE( simpleTypesServiceInB, simple );
    
    simple->setStoredInt( 7 );
    
    EXPECT_EQ( simple->getStoredInt(), simpleTypesServiceInB->getStoredInt() );
    
    setup->tearDown();
}
    
TEST( ParameterTests, complexTypeParameterTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* server = setup->getNode( 1 );
    rpc::INode* client = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> instance = co::newInstance( "stubs.TestComponent" );
    stubs::IComplexTypes* complexTypes = instance->getService<stubs::IComplexTypes>();
    server->publishInstance( instance.get(), "instance" );
    
	// non-published key
	co::RefPtr<co::IObject> rmtInstance;
	EXPECT_NO_THROW(rmtInstance = client->getRemoteInstance("stubs.TestComponent",
                                                                     "notAnInstance", "address1" ) );

	EXPECT_FALSE( rmtInstance.isValid() );

	rmtInstance = client->getRemoteInstance("stubs.TestComponent",
                                                                     "instance", "address1" );
    
    stubs::IComplexTypes* rmtComplexTypes = rmtInstance->getService<stubs::IComplexTypes>();
    
    stubs::MotherStruct ms;
    stubs::ChildStruct cs;    
    stubs::StringNativeClass native;
    
    // setting complex type
    native.data = "setNative";
    cs.myNativeClass = native;
    cs.id = 2;
    cs.name = "setChild";
    cs.anything.set( INT1 );
    ms.id = 1;
    ms.name = "setMother";
    ms.child = cs;

	std::vector<double> doubles( TESTVECSIZE );
	doubles.resize( TESTVECSIZE );
	doubles[0] = 4.56;
	doubles[1] = 6.54;

	ms.doubles = doubles;
    
    rmtComplexTypes->setMotherStruct( ms );
    
    stubs::MotherStruct ms_ = complexTypes->getMotherStruct();
    stubs::ChildStruct cs_ = ms.child;
    EXPECT_EQ( ms_.id, 1 );
    EXPECT_STREQ( ms_.name.c_str(), "setMother" );
    EXPECT_EQ( cs_.id, 2 );
    EXPECT_STREQ( cs_.name.c_str(), "setChild" );
    EXPECT_EQ( cs_.anything.get<co::int32>(), INT1 );

    stubs::StringNativeClass native_ = cs_.myNativeClass;
    
    EXPECT_STREQ( native_.data.c_str(), "setNative" );
    
    // getting complex type
    native.data = "getNative";
    cs.myNativeClass = native;
    cs.id = 4;
    cs.name = "getChild";
    ms.id = 3;
    ms.name = "getMother";
    ms.child = cs;
    
    complexTypes->setMotherStruct( ms );
    
    stubs::MotherStruct ms2_ = rmtComplexTypes->getMotherStruct();
    stubs::ChildStruct cs2_ = ms.child;
    EXPECT_EQ( ms2_.id, 3 );
    EXPECT_STREQ( ms2_.name.c_str(), "getMother" );
    EXPECT_EQ( cs2_.id, 4 );
    EXPECT_STREQ( cs2_.name.c_str(), "getChild" );

	ASSERT_EQ( ms2_.doubles.getKind(), co::TK_ARRAY );

	co::Slice<double> returnedDoubles;
	ASSERT_NO_THROW( returnedDoubles = ms2_.doubles.get<co::Slice<double>>() );
	ASSERT_EQ( returnedDoubles.getSize(), 2 );

	EXPECT_EQ( returnedDoubles[0], 4.56 );
	EXPECT_EQ( returnedDoubles[1], 6.54 );
    
    stubs::StringNativeClass native2_ = cs2_.myNativeClass;
    
    EXPECT_STREQ( native2_.data.c_str(), "getNative" );

    // Structs types inside any
    native.data = "nativeAny";
    cs.myNativeClass = native;
    cs.id = 5;
    cs.name = "childAny";
    ms.child = cs;
    
    co::Any anyMother;
    anyMother.set<const stubs::MotherStruct&>( ms );
    
    co::AnyValue anyChild_ = rmtComplexTypes->getChild( anyMother );
    stubs::ChildStruct cs3_ = anyChild_.get<stubs::ChildStruct&>();
    EXPECT_EQ( cs3_.id, 5 );
    EXPECT_STREQ( cs3_.name.c_str(), "childAny" );
    
    stubs::StringNativeClass native3_ = cs3_.myNativeClass;
    
    EXPECT_STREQ( native3_.data.c_str(), "nativeAny" );
    
    // NativeClass inside any
    co::Any anyNative;
    anyNative.set<const stubs::StringNativeClass&>( native );
    
    co::AnyValue anyNative2_ = rmtComplexTypes->setNativeClassValue( anyNative, "nativeAny2" );
    
    stubs::StringNativeClass native4_ = anyNative2_.get<stubs::StringNativeClass&>();

    EXPECT_STREQ( native4_.data.c_str(), "nativeAny2" );
    
    setup->tearDown();
}
    
TEST( ParameterTests, complexArrayTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* server = setup->getNode( 1 );
    rpc::INode* client = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> instance = co::newInstance( "stubs.TestComponent" );
    server->publishInstance( instance.get(), "instance" );
    
	co::RefPtr<co::IObject> rmtInstance = client->getRemoteInstance("stubs.TestComponent",
                                                                     "instance", "address1" );
    
    stubs::IComplexTypes* remoteCT = rmtInstance->getService<stubs::IComplexTypes>();
    
    
    // After the default initial setup, create the arrays
    std::vector<stubs::MotherStruct> mss; mss.resize( TESTVECSIZE );
    std::vector<stubs::ChildStruct> css; css.resize( TESTVECSIZE );
    std::vector<stubs::StringNativeClass> natives; natives.resize( TESTVECSIZE );

    // Fill the arrays with testable data
    char testString[2];
    testString[1] = '\0';
	std::vector<double> doubles;
	doubles.reserve( TESTVECSIZE );
    for( int i = 0; i < TESTVECSIZE; i++ )
    {
        testString[0] = CHAR1 + i;
        
        // Setting up the Motherstructs array
        mss[i].name = std::string( testString );
        mss[i].id = INT1 + i;
        mss[i].child.id = INT1 + i;
        mss[i].child.name = std::string( testString );
        mss[i].child.myNativeClass.data = std::string( testString );
        mss[i].child.anything.set( INT1 + i );

		doubles.push_back( i );
		mss[i].doubles = doubles;
        
        testString[0] = CHAR2 + i;
        
        // Setting up the ChildStructs Array
        css[i].id = INT2 + i;
        css[i].name = std::string( testString );
        css[i].myNativeClass.data = std::string( testString );
        css[i].anything.set( INT2 + i );
        
        // Setting up the Native classes array
        testString[0] = CHAR3 + i;
        natives[i].data = std::string( testString );
    }
    // Call the methods
    co::TSlice<stubs::MotherStruct> mss_ = remoteCT->placeChilds( mss, css );
    testString[1] = '\0';
    for( int i = 0; i < TESTVECSIZE; i++ )
    {
        testString[0] = CHAR1 + i;
        EXPECT_STREQ( mss_[i].name.c_str(), testString );
        EXPECT_EQ( mss_[i].id , INT1 + i );
        EXPECT_EQ( mss_[i].child.id , INT2 + i );
        testString[0] = CHAR2 + i;
        EXPECT_STREQ( mss_[i].child.name.c_str(), testString );
        EXPECT_STREQ( mss_[i].child.myNativeClass.data.c_str(), testString );
        EXPECT_EQ( mss_[i].child.anything.get<co::int32>(), INT2 + i );

		co::Slice<double> returnedDoubles = mss_[i].doubles.get<co::Slice<double>>();

		for( unsigned int j = 0; j < returnedDoubles.getSize(); j++ )
		{
			EXPECT_DOUBLE_EQ( returnedDoubles[j], j );
		}
    }
   
    co::TSlice<stubs::ChildStruct> css_ = remoteCT->placeNatives( css, natives );
    testString[1] = '\0';
    for( int i = 0; i < TESTVECSIZE; i++ )
    {
        testString[0] = CHAR2 + i;
        EXPECT_STREQ( css_[i].name.c_str(), testString );
        EXPECT_EQ( css_[i].anything.get<co::int32>(), INT2 + i );
        testString[0] = CHAR3 + i;
        EXPECT_STREQ( css_[i].myNativeClass.data.c_str(), testString );
    }

    setup->tearDown();
}
    
} // namespace rpc
