#include <gtest/gtest.h>

#include <stubs/ChildStruct.h>
#include <stubs/MotherStruct.h>
#include <stubs/ISimpleTypes.h>
#include <stubs/IComplexTypes.h>
#include <stubs/IReferenceTypes.h>
#include <stubs/StringNativeClass.h>
#include <stubs/ITestSetup.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>
#include <rpc/RemotingException.h>

#include <co/Log.h>
#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

namespace rpc {

    
TEST( ErrorTests, throwTest )
{
//    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
//    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
//    setup->initTest( 2 );
//    
//    rpc::INode* client = setup->getNode( 1 );
//    rpc::INode* server = setup->getNode( 2 );
//    
//    EXPECT_THROW( client->newRemoteInstance( "invalidComponent", "address2" ), RemotingException );
//    
//    co::IObject* obj = co::newInstance( "stubs.TestComponent" );
//    server->publishInstance( obj, "obj" );
//    
//    EXPECT_THROW( client->findRemoteInstance( "stubs.Incrementer", "obj", "address2" ), 
//                 RemotingException );
//                 
//    co::RefPtr<co::IObject> remoteInstance = client->findRemoteInstance( "stubs.TestComponent", "obj",
//                                                            "address2" );
//    
//    stubs::ISimpleTypes* simple = remoteInstance->getService<stubs::ISimpleTypes>();
//    try
//    {
//        simple->throwException( "rpc.RemotingException", "test" );
//    }
//    catch( std::exception& e )
//    {
//        CORAL_LOG( ERROR ) << "Exception: " << e.what();
//    }
//    
//    EXPECT_THROW( simple->throwException( "rpc.RemotingException", "test" ), RemotingException );
//    
//    setup->tearDown();
}    
} // namespace rpc
