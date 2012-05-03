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
    
TEST( ServantRemovalTests, simpleTest )
{
    co::RefPtr<co::IObject> transportObj = co::newInstance( "mockReef.Transport" );
    reef::ITransport* transport = transportObj->getService<reef::ITransport>();
    
    co::RefPtr<co::IObject> hostANodeObj = co::newInstance( "reef.Node" );
    hostANodeObj->setService( "transport", transport );
    
    co::RefPtr<co::IObject> hostCNodeObj = co::newInstance( "reef.Node" );
    hostCNodeObj->setService( "transport", transport );
    
    co::RefPtr<co::IObject> hostBNodeObj = co::newInstance( "reef.Node" );
    hostBNodeObj->setService( "transport", transport );
    
    reef::INode* hostA = hostANodeObj->getService<reef::INode>();
    reef::INode* hostC = hostCNodeObj->getService<reef::INode>();
    reef::INode* hostB = hostBNodeObj->getService<reef::INode>();
    transportObj->setService( "node", hostA );
    transportObj->setService( "node", hostC );
    transportObj->setService( "node", hostB );
    
    hostA->start( "addressA", "addressA" );    
    
    hostC->start( "addressC", "addressC" );
    
    hostB->start( "addressB", "addressB" );
    
    co::RefVector<co::IObject> objects;
    objects.push_back( hostC->newRemoteInstance( "moduleA.TestComponent", "addressA" ) );
    objects.push_back( hostC->newRemoteInstance( "moduleA.TestComponent", "addressB" ) );
    objects.push_back( hostB->newRemoteInstance( "moduleA.TestComponent", "addressA" ) );
    objects.push_back( hostB->newRemoteInstance( "moduleA.TestComponent", "addressC" ) );
    objects.push_back( hostA->newRemoteInstance( "moduleA.TestComponent", "addressC" ) );
    objects.push_back( hostA->newRemoteInstance( "moduleA.TestComponent", "addressB" ) );
    
    // Each host should have 2 instances with 1 reference each.
    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 1 );
    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 1 );
    EXPECT_EQ( hostA->getRemoteReferences( 3 ), 0 );
    
    for( int i = 0; i < 3; i++ )
    {
        moduleA::ISimpleTypes* st = objects[2 * i + 0]->getService<moduleA::ISimpleTypes>();
        moduleA::IReferenceTypes* rt = objects[2 * i + 1]->getService<moduleA::IReferenceTypes>();
        rt->setSimple( st );
        st = objects[2 * i + 1]->getService<moduleA::ISimpleTypes>();
        rt = objects[2 * i + 0]->getService<moduleA::IReferenceTypes>();
        rt->setSimple( st );
    }
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 2 );
    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 2 );
    EXPECT_EQ( hostA->getRemoteReferences( 3 ), 0 );
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostB->getRemoteReferences( 1 ), 2 );
    EXPECT_EQ( hostB->getRemoteReferences( 2 ), 2 );
    EXPECT_EQ( hostB->getRemoteReferences( 3 ), 0 );
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostC->getRemoteReferences( 1 ), 2 );
    EXPECT_EQ( hostC->getRemoteReferences( 2 ), 2 );
    EXPECT_EQ( hostC->getRemoteReferences( 3 ), 0 );
    
    objects.clear();
    
    // Only need remove 3 instead of 6 references because de other 3 referers will be destroyed
    hostA->getInstance( 1 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
    hostA->getInstance( 2 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
    hostB->getInstance( 2 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
    
    // There should be no more references
    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 0 );
    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 0 );
    EXPECT_EQ( hostB->getRemoteReferences( 1 ), 0 );
    EXPECT_EQ( hostB->getRemoteReferences( 2 ), 0 );
    EXPECT_EQ( hostC->getRemoteReferences( 1 ), 0 );
    EXPECT_EQ( hostC->getRemoteReferences( 2 ), 0 );
}

}