#include <gtest/gtest.h>

#include <moduleA/ISimpleTypes.h>
#include <moduleA/IComplexTypes.h>
#include <moduleA/IReferenceTypes.h>
#include <mockReef/ITestSetup.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace reef {
namespace rpc {

//    
//TEST( InvokerRemovalTests, simpleTest )
//{
//    co::RefPtr<co::IObject> testSetup = co::newInstance( "mockReef.TestSetup" );
//    mockReef::ITestSetup* setup = testSetup->getService<mockReef::ITestSetup>();
//    setup->initTest( 3 );
//    
//    reef::rpc::INode* hostA = setup->getNode( 1 );
//    reef::rpc::INode* hostB = setup->getNode( 2 );
//    reef::rpc::INode* hostC = setup->getNode( 3 );
//    
//    co::RefVector<co::IObject> objects;
//    objects.push_back( hostC->newRemoteInstance( "moduleA.TestComponent", "address1" ) );
//    objects.push_back( hostC->newRemoteInstance( "moduleA.TestComponent", "address2" ) );
//    objects.push_back( hostB->newRemoteInstance( "moduleA.TestComponent", "address1" ) );
//    objects.push_back( hostB->newRemoteInstance( "moduleA.TestComponent", "address3" ) );
//    objects.push_back( hostA->newRemoteInstance( "moduleA.TestComponent", "address3" ) );
//    objects.push_back( hostA->newRemoteInstance( "moduleA.TestComponent", "address2" ) );
//    
//    // Each host should have 2 instances with 1 reference each.
//    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 1 );
//    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 1 );
//    EXPECT_EQ( hostA->getRemoteReferences( 3 ), 0 );
//    
//    for( int i = 0; i < 3; i++ )
//    {
//        moduleA::ISimpleTypes* st = objects[2 * i + 0]->getService<moduleA::ISimpleTypes>();
//        moduleA::IReferenceTypes* rt = objects[2 * i + 1]->getService<moduleA::IReferenceTypes>();
//        rt->setSimple( st );
//        st = objects[2 * i + 1]->getService<moduleA::ISimpleTypes>();
//        rt = objects[2 * i + 0]->getService<moduleA::IReferenceTypes>();
//        rt->setSimple( st );
//    }
//    
//    // Each host should have 2 instances with 2 reference each.
//    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 2 );
//    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 2 );
//    EXPECT_EQ( hostA->getRemoteReferences( 3 ), 0 );
//    
//    // Each host should have 2 instances with 2 reference each.
//    EXPECT_EQ( hostB->getRemoteReferences( 1 ), 2 );
//    EXPECT_EQ( hostB->getRemoteReferences( 2 ), 2 );
//    EXPECT_EQ( hostB->getRemoteReferences( 3 ), 0 );
//    
//    // Each host should have 2 instances with 2 reference each.
//    EXPECT_EQ( hostC->getRemoteReferences( 1 ), 2 );
//    EXPECT_EQ( hostC->getRemoteReferences( 2 ), 2 );
//    EXPECT_EQ( hostC->getRemoteReferences( 3 ), 0 );
//    
//    objects.clear();
//    
//    // Only need remove 3 instead of 6 references because de other 3 referers will be destroyed
//    hostA->getInstance( 1 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
//    hostA->getInstance( 2 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
//    hostB->getInstance( 2 )->getService<moduleA::IReferenceTypes>()->setSimple( 0 );
//    
//    // There should be no more references
//    EXPECT_EQ( hostA->getRemoteReferences( 1 ), 0 );
//    EXPECT_EQ( hostA->getRemoteReferences( 2 ), 0 );
//    EXPECT_EQ( hostB->getRemoteReferences( 1 ), 0 );
//    EXPECT_EQ( hostB->getRemoteReferences( 2 ), 0 );
//    EXPECT_EQ( hostC->getRemoteReferences( 1 ), 0 );
//    EXPECT_EQ( hostC->getRemoteReferences( 2 ), 0 );
//    
//    setup->tearDown();
//}
//    
}
    
}