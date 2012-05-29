#include <gtest/gtest.h>

#include <moduleA/ISimpleTypes.h>
#include <moduleA/IComplexTypes.h>
#include <moduleA/IReferenceTypes.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>

namespace reef {
namespace rpc {

    
TEST( SmokeTests, simpleTypesTest )
{
    // Creates the node instance
    co::IObject* nodeObj = co::newInstance( "reef.rpc.Node" );
    
    // Gets the INode interface, which is the interface with remote hosts
    reef::rpc::INode* node = nodeObj->getService<reef::rpc::INode>();
    
    // Creates the instance responsible for the transport layer
    reef::rpc::ITransport* transport = co::newInstance( "zmq.ZMQTransport" )->getService<reef::rpc::ITransport>();
    
    // The node instance needs the transport layer to communicate
    nodeObj->setService( "transport", transport );
    
    /* Even though this node wont be acting as a server, it is necessary to be bound to a public
     address. This happens because whenever a local service is passed as a reference argument, 
     the receiver will need to send messages to this node. */
    node->start( "tcp://*:4021", "tcp://localhost:4021" );
    
    // instantiates a moduleA.TestComponent in host bound to the given address
    co::RefPtr<co::IObject> remoteInstance = node->newRemoteInstance( "moduleA.TestComponent",
                                                                       "tcp://localhost:4020" );
    
    // Gets the moduleA.ISimpleTypes interface from the remote instance
    moduleA::ISimpleTypes* simple = remoteInstance->getService<moduleA::ISimpleTypes>();
    
    // simple get and set test
    simple->setDouble( 0.1 );
    EXPECT_EQ( simple->getStoredDouble(), 0.1 );
}
    
}
}
