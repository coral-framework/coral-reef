#include <gtest/gtest.h>

#include <moduleA/ChildStruct.h>
#include <moduleA/MotherStruct.h>
#include <moduleA/ISimpleTypes.h>
#include <moduleA/IComplexTypes.h>
#include <moduleA/IReferenceTypes.h>
#include <moduleA/StringNativeClass.h>
#include <mockReef/ITestSetup.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace reef {
namespace rpc {

    
TEST( ParameterTests, simpleTypesTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "mockReef.TestSetup" );
    mockReef::ITestSetup* setup = testSetup->getService<mockReef::ITestSetup>();
    setup->initTest( 2 );
    
    reef::rpc::INode* client = setup->getNode( 1 );
    
    co::RefPtr<co::IObject> remoteInstance = client->newRemoteInstance( "moduleA.TestComponent",
                                                            "address2" );
    moduleA::ISimpleTypes* simple = remoteInstance->getService<moduleA::ISimpleTypes>();
    
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
    co::RefPtr<co::IObject> testSetup = co::newInstance( "mockReef.TestSetup" );
    mockReef::ITestSetup* setup = testSetup->getService<mockReef::ITestSetup>();
    setup->initTest( 3 );
    
    reef::rpc::INode* serverB = setup->getNode( 2 );
    reef::rpc::INode* client = setup->getNode( 3 );

    co::RefPtr<co::IObject> instanceInA = client->newRemoteInstance( "moduleA.TestComponent",
                                                                "address1" );
    moduleA::IReferenceTypes* refTypesServiceInA = instanceInA->getService<moduleA::IReferenceTypes>();
    moduleA::ISimpleTypes* simpleTypesServiceInA = instanceInA->getService<moduleA::ISimpleTypes>();
    
    co::RefPtr<co::IObject> instanceInB = client->newRemoteInstance( "moduleA.TestComponent",
                                                                    "address2" );
    moduleA::ISimpleTypes* simpleTypesServiceInB = instanceInB->getService<moduleA::ISimpleTypes>();

    
    co::RefPtr<co::IObject> instanceLocal = co::newInstance( "moduleA.TestComponent" );
    moduleA::ISimpleTypes* simpleTypesServiceLocal = instanceLocal->getService<moduleA::ISimpleTypes>();
    
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
    
    moduleA::ISimpleTypes* simple = serverB->getInstance( 1 )->getService<moduleA::ISimpleTypes>();
    
    EXPECT_NE( simpleTypesServiceInB, simple );
    
    simple->setStoredInt( 7 );
    
    EXPECT_EQ( simple->getStoredInt(), simpleTypesServiceInB->getStoredInt() );
    
    setup->tearDown();
}
    
TEST( ParameterTests, complexTypeParameterTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "mockReef.TestSetup" );
    mockReef::ITestSetup* setup = testSetup->getService<mockReef::ITestSetup>();
    setup->initTest( 2 );
    
    reef::rpc::INode* server = setup->getNode( 1 );
    reef::rpc::INode* client = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> instance = co::newInstance( "moduleA.TestComponent" );
    moduleA::IComplexTypes* complexTypes = instance->getService<moduleA::IComplexTypes>();
    server->publishInstance( instance.get(), "instance" );
    
    co::RefPtr<co::IObject> rmtInstance = client->findRemoteInstance( "moduleA.TestComponent", 
                                                                     "instance", "address1" );
    
    moduleA::IComplexTypes* rmtComplexTypes = rmtInstance->getService<moduleA::IComplexTypes>();
    
    moduleA::MotherStruct ms;
    moduleA::ChildStruct cs;    
    moduleA::StringNativeClass native;
    
    // setting complex type
    native.data = "setNative";
    cs.myNativeClass = native;
    cs.id = 2;
    cs.name = "setChild";
    ms.id = 1;
    ms.name = "setMother";
    ms.child = cs;
    
    rmtComplexTypes->setMotherStruct( ms );
    
    const moduleA::MotherStruct& ms_ = complexTypes->getMotherStruct();
    const moduleA::ChildStruct& cs_ = ms.child;
    EXPECT_EQ( ms_.id, 1 );
    EXPECT_STREQ( ms_.name.c_str(), "setMother" );
    EXPECT_EQ( cs_.id, 2 );
    EXPECT_STREQ( cs_.name.c_str(), "setChild" );

    const moduleA::StringNativeClass& native_ = cs_.myNativeClass;
    
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
    
    const moduleA::MotherStruct& ms2_ = rmtComplexTypes->getMotherStruct();
    const moduleA::ChildStruct& cs2_ = ms.child;
    EXPECT_EQ( ms2_.id, 3 );
    EXPECT_STREQ( ms2_.name.c_str(), "getMother" );
    EXPECT_EQ( cs2_.id, 4 );
    EXPECT_STREQ( cs2_.name.c_str(), "getChild" );
    
    const moduleA::StringNativeClass& native2_ = cs2_.myNativeClass;
    
    EXPECT_STREQ( native2_.data.c_str(), "getNative" );

    
    setup->tearDown();
}

}

}
