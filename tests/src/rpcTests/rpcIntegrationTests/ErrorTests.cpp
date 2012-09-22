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

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace rpc {

    
TEST( ErrorTests, throwTest )
{
    co::RefPtr<co::IObject> testSetup = co::newInstance( "stubs.TestSetup" );
    stubs::ITestSetup* setup = testSetup->getService<stubs::ITestSetup>();
    setup->initTest( 2 );
    
    rpc::INode* client = setup->getNode( 1 );
    
    co::RefPtr<co::IObject> remoteInstance = client->newRemoteInstance( "stubs.TestComponent",
                                                            "address2" );
    stubs::ISimpleTypes* simple = remoteInstance->getService<stubs::ISimpleTypes>();
    
    EXPECT_THROW( simple->throwException( "rpc.RemotingException", "test" ), RemotingException );
    
}    
} // namespace rpc
