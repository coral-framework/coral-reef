#include <gtest/gtest.h>

#include <rpcTests/ChildStruct.h>
#include <rpcTests/MotherStruct.h>
#include <rpcTests/ISimpleTypes.h>
#include <rpcTests/IComplexTypes.h>
#include <rpcTests/IReferenceTypes.h>
#include <rpcTests/StringNativeClass.h>
#include <rpcTests/ITestSetup.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>
#include <rpc/RemotingException.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace rpc {

    
TEST( ErrorTests, throwTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "rpcTests.TestSetup" );
    rpcTests::ITestSetup* setup = testSetup->getService<rpcTests::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* client = setup->getNode( 1 );
    
    co::RefPtr<co::IObject> remoteInstance = client->newRemoteInstance( "rpcTests.TestComponent",
                                                            "address2" );
    rpcTests::ISimpleTypes* simple = remoteInstance->getService<rpcTests::ISimpleTypes>();
    
    EXPECT_THROW( simple->throwException( "rpc.RemotingException", "test" ), RemotingException );
    
}    
} // namespace rpc
