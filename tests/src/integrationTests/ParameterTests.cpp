#include <gtest/gtest.h>

#include <moduleA/ISimpleTypes.h>
#include <moduleA/IComplexTypes.h>
#include <moduleA/IReferenceTypes.h>

#include <reef/INode.h>
#include <reef/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace reef
{
    
TEST( SmokeTests, simpleTypesTest )
{
    co::RefPtr<co::IObject> transportObj = co::newInstance( "mockReef.Transport" );
    reef::ITransport* transport = transportObj->getService<reef::ITransport>();
    
    co::RefPtr<co::IObject> serverNodeObj = co::newInstance( "reef.Node" );
    serverNodeObj->setService( "transport", transport );
    
    co::RefPtr<co::IObject> cliNodeObj = co::newInstance( "reef.Node" );
    cliNodeObj->setService( "transport", transport );
    
    reef::INode* server = serverNodeObj->getService<reef::INode>();
    reef::INode* client = cliNodeObj->getService<reef::INode>();
    transportObj->setService( "node", server );
    transportObj->setService( "node", client );
    
    server->start( "address1", "address1" );    
    
    client->start( "address2", "address2" );
    
    co::RefPtr<co::IObject> remoteInstance = client->newRemoteInstance( "moduleA.TestComponent",
                                                            "address1" );
    moduleA::ISimpleTypes* simple = remoteInstance->getService<moduleA::ISimpleTypes>();
    
    simple->setDouble( 0.1 );
    EXPECT_EQ( simple->getStoredDouble(), 0.1 );
    
    simple->setStoredInt( 99 );
    EXPECT_EQ( simple->getStoredInt(), 99 );
    
    EXPECT_EQ( simple->divide( 10, 5), 2.0 );
    
    const std::string testString( "1234" );
    simple->setString( testString );
    EXPECT_STREQ( testString.c_str() , simple->getStoredString().c_str() );
    
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
    simple->setStoredStringList( stringVec );
    simple->setStoredDoubleList( doubleVec );
    
    co::Range<const co::int32> intRange = simple->getStoredIntList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( intVec[i], intRange[i] );
    
    co::Range<const double> doubleRange = simple->getStoredDoubleList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( doubleVec[i], doubleRange[i] );
    
    co::Range<const std::string> stringRange = simple->getStoredStringList();
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
}

TEST( SmokeTests, refTypeParameterTest )
{
    co::RefPtr<co::IObject> transportObj = co::newInstance( "mockReef.Transport" );
    reef::ITransport* transport = transportObj->getService<reef::ITransport>();
    
    co::RefPtr<co::IObject> serverANodeObj = co::newInstance( "reef.Node" );
    serverANodeObj->setService( "transport", transport );
    
    co::RefPtr<co::IObject> cliNodeObj = co::newInstance( "reef.Node" );
    cliNodeObj->setService( "transport", transport );
    
    co::RefPtr<co::IObject> serverBNodeObj = co::newInstance( "reef.Node" );
    serverBNodeObj->setService( "transport", transport );
    
    reef::INode* serverA = serverANodeObj->getService<reef::INode>();
    reef::INode* client = cliNodeObj->getService<reef::INode>();
    reef::INode* serverB = serverBNodeObj->getService<reef::INode>();
    transportObj->setService( "node", serverA );
    transportObj->setService( "node", client );
    transportObj->setService( "node", serverB );
    
    serverA->start( "addressA", "addressA" );    
    
    client->start( "addressClient", "addressClient" );
    
    serverB->start( "addressB", "addressB" );


    co::RefPtr<co::IObject> instanceInA = client->newRemoteInstance( "moduleA.TestComponent",
                                                                "addressA" );
    moduleA::IReferenceTypes* refTypesServiceInA = instanceInA->getService<moduleA::IReferenceTypes>();
    moduleA::ISimpleTypes* simpleTypesServiceInA = instanceInA->getService<moduleA::ISimpleTypes>();
    
    co::RefPtr<co::IObject> instanceInB = client->newRemoteInstance( "moduleA.TestComponent",
                                                                    "addressB" );
    moduleA::ISimpleTypes* simpleTypesServiceInB = instanceInB->getService<moduleA::ISimpleTypes>();

    
    co::RefPtr<co::IObject> instanceLocal = co::newInstance( "moduleA.TestComponent" );
    moduleA::ISimpleTypes* simpleTypesServiceLocal = instanceLocal->getService<moduleA::ISimpleTypes>();
    
    EXPECT_EQ( refTypesServiceInA->callIncrementInt( simpleTypesServiceInA, 3 ), 4 );
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceInA, 15, 5 ), 3 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceInA, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    EXPECT_EQ( refTypesServiceInA->callIncrementInt( simpleTypesServiceLocal, 3 ), 4 );
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceLocal, 15, 5 ), 3 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceLocal, "aaa", "bbb" ).c_str(), "aaabbb" );
    
    
    EXPECT_EQ( refTypesServiceInA->callIncrementInt( simpleTypesServiceInB, 3 ), 4 );
    EXPECT_EQ( refTypesServiceInA->callDivideDouble( simpleTypesServiceInB, 15, 5 ), 3 );
    EXPECT_STREQ( refTypesServiceInA->concatenateString( simpleTypesServiceInB, "aaa", "bbb" ).c_str(), "aaabbb" );

}

    
}
