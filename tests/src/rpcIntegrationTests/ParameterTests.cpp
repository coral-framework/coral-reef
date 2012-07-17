#include <gtest/gtest.h>

#include <rpcTests/ChildStruct.h>
#include <rpcTests/MotherStruct.h>
#include <rpcTests/ISimpleTypes.h>
#include <rpcTests/IComplexTypes.h>
#include <rpcTests/IReferenceTypes.h>
#include <rpcTests/StringNativeClass.h>
#include <rpcTests/ITestSetup.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

#define INT1 10
#define INT2 20
#define INT3 30
#define INT4 40
#define BOOL1 true
#define DOUBLE1 10.0
#define STRING1 "string1"
#define STRING2 "string2"
#define TESTVECSIZE 10
#define CHAR1 65
#define CHAR2 75
#define CHAR3 85

namespace reef {
namespace rpc {

    
TEST( ParameterTests, simpleTypesTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "rpcTests.TestSetup" );
    rpcTests::ITestSetup* setup = testSetup->getService<rpcTests::ITestSetup>();
    setup->initTest( 2 );
    
    reef::rpc::INode* client = setup->getNode( 1 );
    
    co::RefPtr<co::IObject> remoteInstance = client->newRemoteInstance( "rpcTests.TestComponent",
                                                            "address2" );
    rpcTests::ISimpleTypes* simple = remoteInstance->getService<rpcTests::ISimpleTypes>();
    
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
    co::Any d1; d1.set<double>( 4 );
    const co::Any& dResult = simple->addDoublesFromAny( d1, d1 ); // Cannot pass different anys now
    EXPECT_DOUBLE_EQ( 8, dResult.get<double>() );
    
    co::Any str1Any; 
    std::string& str1 = str1Any.createString();
    str1 = "test";
    const co::Any& strResult = simple->concatenateFromAny( str1Any, str1Any );
    EXPECT_STREQ( "testtest", strResult.get<const std::string&>().c_str() );
    
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
    
    co::Range<const co::int32> intRange = simple->getStoredIntList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( intVec[i], intRange[i] );
    
    co::Range<const double> doubleRange = simple->parentMergeLists( doubleVec, doubleVec );
    int size = doubleRange.getSize();

    for( int i = 0; i < size; i++ )
    {
        double comparison = i % 10;
        double value = doubleRange[i];
        EXPECT_DOUBLE_EQ( comparison, value );
    }
    
    co::Range<const std::string> stringRange = simple->getParentStringList();
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
    stringRange = simple->getThirdElements( stringVec, stringVec2 );
    
    EXPECT_STREQ( stringRange[0].c_str(), stringVec[2].c_str() );
    EXPECT_STREQ( stringRange[1].c_str(), stringVec2[2].c_str() );
    
    setup->tearDown();
}

TEST( ParameterTests, refTypeParameterTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "rpcTests.TestSetup" );
    rpcTests::ITestSetup* setup = testSetup->getService<rpcTests::ITestSetup>();
    setup->initTest( 3 );
    
    reef::rpc::INode* serverB = setup->getNode( 2 );
    reef::rpc::INode* client = setup->getNode( 3 );

    co::RefPtr<co::IObject> instanceInA = client->newRemoteInstance( "rpcTests.TestComponent",
                                                                "address1" );
    rpcTests::IReferenceTypes* refTypesServiceInA = instanceInA->getService<rpcTests::IReferenceTypes>();
    rpcTests::ISimpleTypes* simpleTypesServiceInA = instanceInA->getService<rpcTests::ISimpleTypes>();
    
    co::RefPtr<co::IObject> instanceInB = client->newRemoteInstance( "rpcTests.TestComponent",
                                                                    "address2" );
    rpcTests::ISimpleTypes* simpleTypesServiceInB = instanceInB->getService<rpcTests::ISimpleTypes>();

    
    co::RefPtr<co::IObject> instanceLocal = co::newInstance( "rpcTests.TestComponent" );
    rpcTests::ISimpleTypes* simpleTypesServiceLocal = instanceLocal->getService<rpcTests::ISimpleTypes>();
    
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
    EXPECT_EQ( simpleTypesServiceInB, refTypesServiceInA->getSimple() );
    
    rpcTests::ISimpleTypes* simple = serverB->getInstance( 0 )->getService<rpcTests::ISimpleTypes>();
    
    EXPECT_NE( simpleTypesServiceInB, simple );
    
    simple->setStoredInt( 7 );
    
    EXPECT_EQ( simple->getStoredInt(), simpleTypesServiceInB->getStoredInt() );
    
    setup->tearDown();
}
    
TEST( ParameterTests, complexTypeParameterTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "rpcTests.TestSetup" );
    rpcTests::ITestSetup* setup = testSetup->getService<rpcTests::ITestSetup>();
    setup->initTest( 2 );
    
    reef::rpc::INode* server = setup->getNode( 1 );
    reef::rpc::INode* client = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> instance = co::newInstance( "rpcTests.TestComponent" );
    rpcTests::IComplexTypes* complexTypes = instance->getService<rpcTests::IComplexTypes>();
    server->publishInstance( instance.get(), "instance" );
    
    co::RefPtr<co::IObject> rmtInstance = client->findRemoteInstance( "rpcTests.TestComponent", 
                                                                     "instance", "address1" );
    
    rpcTests::IComplexTypes* rmtComplexTypes = rmtInstance->getService<rpcTests::IComplexTypes>();
    
    rpcTests::MotherStruct ms;
    rpcTests::ChildStruct cs;    
    rpcTests::StringNativeClass native;
    
    // setting complex type
    native.data = "setNative";
    cs.myNativeClass = native;
    cs.id = 2;
    cs.name = "setChild";
    cs.anything.set<co::int32>( INT1 );
    ms.id = 1;
    ms.name = "setMother";
    ms.child = cs;
    
    rmtComplexTypes->setMotherStruct( ms );
    
    rpcTests::MotherStruct ms_ = complexTypes->getMotherStruct();
    rpcTests::ChildStruct cs_ = ms.child;
    EXPECT_EQ( ms_.id, 1 );
    EXPECT_STREQ( ms_.name.c_str(), "setMother" );
    EXPECT_EQ( cs_.id, 2 );
    EXPECT_STREQ( cs_.name.c_str(), "setChild" );
    EXPECT_EQ( cs_.anything.get<co::int32>(), INT1 );

    rpcTests::StringNativeClass native_ = cs_.myNativeClass;
    
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
    
    rpcTests::MotherStruct ms2_ = rmtComplexTypes->getMotherStruct();
    rpcTests::ChildStruct cs2_ = ms.child;
    EXPECT_EQ( ms2_.id, 3 );
    EXPECT_STREQ( ms2_.name.c_str(), "getMother" );
    EXPECT_EQ( cs2_.id, 4 );
    EXPECT_STREQ( cs2_.name.c_str(), "getChild" );
    
    rpcTests::StringNativeClass native2_ = cs2_.myNativeClass;
    
    EXPECT_STREQ( native2_.data.c_str(), "getNative" );

    // Structs types inside any
    native.data = "nativeAny";
    cs.myNativeClass = native;
    cs.id = 5;
    cs.name = "childAny";
    ms.child = cs;
    
    co::Any anyMother;
    anyMother.set<const rpcTests::MotherStruct&>( ms );
    
    co::Any anyChild_ = rmtComplexTypes->getChild( anyMother );
    rpcTests::ChildStruct cs3_ = anyChild_.get<const rpcTests::ChildStruct&>();
    EXPECT_EQ( cs3_.id, 5 );
    EXPECT_STREQ( cs3_.name.c_str(), "childAny" );
    
    rpcTests::StringNativeClass native3_ = cs3_.myNativeClass;
    
    EXPECT_STREQ( native3_.data.c_str(), "nativeAny" );
    
    // NativeClass inside any
    co::Any anyNative;
    anyNative.set<const rpcTests::StringNativeClass&>( native );
    
    co::Any anyNative2_ = rmtComplexTypes->setNativeClassValue( anyNative, "nativeAny2" );
    
    rpcTests::StringNativeClass native4_ = anyNative2_.get<const rpcTests::StringNativeClass&>();

    EXPECT_STREQ( native4_.data.c_str(), "nativeAny2" );
    
    setup->tearDown();
}
    
TEST( ParameterTests, complexArrayTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "rpcTests.TestSetup" );
    rpcTests::ITestSetup* setup = testSetup->getService<rpcTests::ITestSetup>();
    setup->initTest( 2 );
    
    reef::rpc::INode* server = setup->getNode( 1 );
    reef::rpc::INode* client = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> instance = co::newInstance( "rpcTests.TestComponent" );
    server->publishInstance( instance.get(), "instance" );
    
    co::RefPtr<co::IObject> rmtInstance = client->findRemoteInstance( "rpcTests.TestComponent", 
                                                                     "instance", "address1" );
    
    rpcTests::IComplexTypes* remoteCT = rmtInstance->getService<rpcTests::IComplexTypes>();
    
    
    // After the default initial setup, create the arrays
    std::vector<rpcTests::MotherStruct> mss; mss.resize( TESTVECSIZE );
    std::vector<rpcTests::ChildStruct> css; css.resize( TESTVECSIZE );
    std::vector<rpcTests::StringNativeClass> natives; natives.resize( TESTVECSIZE );

    // Fill the arrays with testable data
    char testString[2];
    testString[1] = '\0';
    for( int i = 0; i < TESTVECSIZE; i++ )
    {
        testString[0] = CHAR1 + i;
        
        // Setting up the Motherstructs array
        mss[i].name = std::string( testString );
        mss[i].id = INT1 + i;
        mss[i].child.id = INT1 + i;
        mss[i].child.name = std::string( testString );
        mss[i].child.myNativeClass.data = std::string( testString );
        mss[i].child.anything.set<co::int32>( INT1 + i );
        
        testString[0] = CHAR2 + i;
        
        // Setting up the ChildStructs Array
        css[i].id = INT2 + i;
        css[i].name = std::string( testString );
        css[i].myNativeClass.data = std::string( testString );
        css[i].anything.set<co::int32>( INT2 + i );
        
        // Setting up the Native classes array
        testString[0] = CHAR3 + i;
        natives[i].data = std::string( testString );
    }
    
    // Call the methods
    co::Range<const rpcTests::MotherStruct> mss_ = remoteCT->placeChilds( mss, css );
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
    }
    
    co::Range<const rpcTests::ChildStruct> css_ = remoteCT->placeNatives( css, natives );
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
    
}

}
