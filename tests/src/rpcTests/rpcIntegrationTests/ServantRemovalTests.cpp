#include <gtest/gtest.h>

#include <stubs/ISimpleTypes.h>
#include <stubs/IComplexTypes.h>
#include <stubs/IReferenceTypes.h>
#include <stubs/ITestSetup.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace rpc {

    
TEST( InvokerRemovalTests, simpleTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 3 );
    
    rpc::INode* hostA = setup->getNode( 1 );
    rpc::INode* hostB = setup->getNode( 2 );
    rpc::INode* hostC = setup->getNode( 3 );
    
    co::RefVector<co::IObject> objects;
    objects.push_back( hostC->newRemoteInstance( "stubs.TestComponent", "address1" ) );
    objects.push_back( hostC->newRemoteInstance( "stubs.TestComponent", "address2" ) );
    objects.push_back( hostB->newRemoteInstance( "stubs.TestComponent", "address1" ) );
    objects.push_back( hostB->newRemoteInstance( "stubs.TestComponent", "address3" ) );
    objects.push_back( hostA->newRemoteInstance( "stubs.TestComponent", "address3" ) );
    objects.push_back( hostA->newRemoteInstance( "stubs.TestComponent", "address2" ) );
    
    // Each host should have 2 instances with 1 reference each.
    EXPECT_EQ( hostA->getInstanceNumLeases( 0 ), 1 );
    EXPECT_EQ( hostA->getInstanceNumLeases( 1 ), 1 );
    EXPECT_EQ( hostA->getInstanceNumLeases( 2 ), 0 );
    
    for( int i = 0; i < 3; i++ )
    {
        stubs::ISimpleTypes* st = objects[2 * i + 0]->getService<stubs::ISimpleTypes>();
        stubs::IReferenceTypes* rt = objects[2 * i + 1]->getService<stubs::IReferenceTypes>();
        rt->setSimple( st );
        st = objects[2 * i + 1]->getService<stubs::ISimpleTypes>();
        rt = objects[2 * i + 0]->getService<stubs::IReferenceTypes>();
        rt->setSimple( st );
    }
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostA->getInstanceNumLeases( 0 ), 2 );
    EXPECT_EQ( hostA->getInstanceNumLeases( 1 ), 2 );
    EXPECT_EQ( hostA->getInstanceNumLeases( 2 ), 0 );
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostB->getInstanceNumLeases( 0 ), 2 );
    EXPECT_EQ( hostB->getInstanceNumLeases( 1 ), 2 );
    EXPECT_EQ( hostB->getInstanceNumLeases( 2 ), 0 );
    
    // Each host should have 2 instances with 2 reference each.
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 2 );
    EXPECT_EQ( hostC->getInstanceNumLeases( 1 ), 2 );
    EXPECT_EQ( hostC->getInstanceNumLeases( 2 ), 0 );
    
    objects.clear();
    
    // Only need remove 3 instead of 6 references because de other 3 referers will be destroyed
    hostA->getInstance( 0 )->getService<stubs::IReferenceTypes>()->setSimple( 0 );
    hostA->getInstance( 1 )->getService<stubs::IReferenceTypes>()->setSimple( 0 );
    hostB->getInstance( 1 )->getService<stubs::IReferenceTypes>()->setSimple( 0 );
    
    // There should be no more references
    EXPECT_EQ( hostA->getInstanceNumLeases( 1 ), 0 );
    EXPECT_EQ( hostA->getInstanceNumLeases( 2 ), 0 );
    EXPECT_EQ( hostB->getInstanceNumLeases( 1 ), 0 );
    EXPECT_EQ( hostB->getInstanceNumLeases( 2 ), 0 );
    EXPECT_EQ( hostC->getInstanceNumLeases( 1 ), 0 );
    EXPECT_EQ( hostC->getInstanceNumLeases( 2 ), 0 );
    
    setup->tearDown();
}

TEST( InvokerRemovalTests, repeatedReferenceTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 3 );
    
    rpc::INode* hostA = setup->getNode( 1 );
    rpc::INode* hostC = setup->getNode( 3 );
    
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 0 );
    
    co::RefPtr<co::IObject> pAtoC = hostA->newRemoteInstance( "stubs.TestComponent", "address3" );
    co::RefPtr<co::IObject> pA1toB = hostA->newRemoteInstance( "stubs.TestComponent", "address2" );
    co::RefPtr<co::IObject> pA2toB = hostA->newRemoteInstance( "stubs.TestComponent", "address2" );
    
    stubs::ISimpleTypes* st = pAtoC->getService<stubs::ISimpleTypes>();
    stubs::IReferenceTypes* rt1 = pA1toB->getService<stubs::IReferenceTypes>();
    stubs::IReferenceTypes* rt2 = pA2toB->getService<stubs::IReferenceTypes>();
    
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 1 );
    
    rt1->setSimple( st );
    
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 2 );
    
    rt2->setSimple( st );
    
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 2 );
    
    pAtoC = 0;
    pA1toB = 0;
    pA2toB = 0;
    
    EXPECT_EQ( hostC->getInstanceNumLeases( 0 ), 0 );
    
    setup->tearDown();
}

TEST( InvokerRemovalTests, unpublishingTests )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* hostA = setup->getNode( 1 );
    rpc::INode* hostB = setup->getNode( 2 );
    
    co::RefPtr<co::IObject> obj = co::newInstance( "stubs.TestComponent" );
    hostA->publishInstance( obj.get(), "key" );
    
    co::RefPtr<stubs::ISimpleTypes> st = hostB->findRemoteInstance( "stubs.TestComponent", "key", 
                                                "address1" )->getService<stubs::ISimpleTypes>();
    
    EXPECT_EQ( hostA->getInstanceNumLeases( 0 ), 2 );
    
    EXPECT_EQ( st->incrementInt( 1 ), 2 );
    
    hostA->unpublishInstance( "key" );
    
    EXPECT_EQ( st->incrementInt( 1 ), 2 );
    
    EXPECT_EQ( hostA->getInstanceNumLeases( 0 ), 1 );
    
    st = 0;
    
    EXPECT_EQ( hostA->getInstanceNumLeases( 0 ), 0 );
}
    
} // namespace rpc