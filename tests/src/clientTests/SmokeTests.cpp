#include <gtest/gtest.h>

#include <testModule/ISimpleTypes.h>
#include <testModule/IComplexTypes.h>
#include <testModule/IReferenceTypes.h>

#include <reef/INode.h>
#include <reef/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

namespace reef
{

//TEST( SmokeTests, simpleTypesTest )
//{
//    co::IObject* nodeObj = co::newInstance( "reef.Node" );
//    reef::INode* node = nodeObj->getService<reef::INode>();
//    reef::ITransport* transport = co::newInstance( "reef.ZMQTransport" )->getService<reef::ITransport>();
//    nodeObj->setService( "transport", transport );
//    
//    node->start( "tcp://*:4021", "tcp://localhost:4021" );
//    
//    testModule::ISimpleTypes* simple = node->newRemoteInstance( "testModule.TestComponent",
//                                    "tcp://localhost:4020" )->getService<testModule::ISimpleTypes>();
//    
//    simple->setDouble( 0.1 );
//    EXPECT_EQ( simple->getStoredDouble(), 0.1 );
//    
//    simple->setStoredInt( 99 );
//    EXPECT_EQ( simple->getStoredInt(), 99 );
//    
//    EXPECT_EQ( simple->divide( 10, 5), 2.0 );
//    
//    const std::string testString( "1234" );
//    simple->setString( testString );
//    EXPECT_STREQ( testString.c_str() , simple->getStoredString().c_str() );
//    
//    std::vector<co::int32> intVec;
//    std::vector<double> doubleVec;
//    std::vector<std::string> stringVec;
//    for( int i = 0; i < 10; i++ )
//    {
//        char letter[2];
//		letter[0] = 65 + i;
//		letter[1] = '\0';
//        stringVec.push_back( letter );
//        intVec.push_back( i );
//        doubleVec.push_back( static_cast<double>( i ) );
//    }
//    
//    simple->setStoredIntList( intVec );
//    simple->setStoredStringList( stringVec );
//    simple->setStoredDoubleList( doubleVec );
//    
//    co::Range<const co::int32> intRange = simple->getStoredIntList();
//    for( int i = 0; i < intVec.size(); i++ )
//        EXPECT_EQ( intVec[i], intRange[i] );
//    
//    co::Range<const double> doubleRange = simple->getStoredDoubleList();
//    for( int i = 0; i < intVec.size(); i++ )
//        EXPECT_EQ( doubleVec[i], doubleRange[i] );
//    
//    co::Range<const std::string> stringRange = simple->getStoredStringList();
//    for( int i = 0; i < intVec.size(); i++ )
//        EXPECT_STREQ( stringVec[i].c_str(), stringRange[i].c_str() );
//    
//    
//    EXPECT_EQ( simple->incrementInt( 1 ), 2 );
//    EXPECT_DOUBLE_EQ( simple->divide( 10, 5 ), 2 );
//    EXPECT_STREQ( "abcdef", simple->concatenateString( "abc", "def" ).c_str() );
//    
//    std::vector<std::string> stringVec2;
//    for( int i = 0; i < 10; i++ )
//    {
//        char letter[2];
//		letter[0] = 21 + i;
//		letter[1] = '\0';
//        stringVec2.push_back( letter );
//    }
//    stringRange = simple->getThirdElements( stringVec, stringVec2 );
//    
//    EXPECT_STREQ( stringRange[0].c_str(), stringVec[2].c_str() );
//    EXPECT_STREQ( stringRange[1].c_str(), stringVec2[2].c_str() );
//}

TEST( SmokeTests, refTypeParameterTest )
{
    co::IObject* nodeObj = co::newInstance( "reef.Node" );
    reef::INode* node = nodeObj->getService<reef::INode>();
    reef::ITransport* transport = co::newInstance( "reef.ZMQTransport" )->getService<reef::ITransport>();
    nodeObj->setService( "transport", transport );
    node->start( "tcp://*:4021", "tcp://localhost:4021" );

    co::RefPtr<co::IObject> remoteTC = node->newRemoteInstance( "testModule.TestComponent",
                                                                "tcp://localhost:4020" );
    testModule::ISimpleTypes* remoteSimple = remoteTC->getService<testModule::ISimpleTypes>();
    testModule::IReferenceTypes* remoteRef = remoteTC->getService<testModule::IReferenceTypes>();
    
    co::RefPtr<co::IObject> localTC = co::newInstance( "testModule.TestComponent" );
    testModule::ISimpleTypes* localSimple = localTC->getService<testModule::ISimpleTypes>();
    testModule::IReferenceTypes* localRef = localTC->getService<testModule::IReferenceTypes>();
    
    EXPECT_EQ( remoteRef->callIncrementInt( remoteSimple, 3 ), 4 );
    EXPECT_EQ( remoteRef->callDivideDouble( remoteSimple, 15, 5 ), 3 );
    EXPECT_STREQ( remoteRef->concatenateString( remoteSimple, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    EXPECT_EQ( remoteRef->callIncrementInt( localSimple, 3 ), 4 );
    EXPECT_EQ( remoteRef->callDivideDouble( localSimple, 15, 5 ), 3 );
    EXPECT_STREQ( remoteRef->concatenateString( localSimple, "aaa", "bbb" ).c_str(), "aaabbb" );

}

    
}
