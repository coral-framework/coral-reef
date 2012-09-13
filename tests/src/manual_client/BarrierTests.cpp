#include <gtest/gtest.h>

#include <rpcTests/ISimpleTypes.h>
#include <rpcTests/IIncrementer.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <dso/IClientSpace.h>

#include <dom/ICompany.h>
#include <dom/IEmployee.h>
#include <dom/IService.h>
#include <dom/IProduct.h>

#include <ca/IModel.h>
#include <ca/ISpace.h>
#include <ca/IUniverse.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>
#include <co/RefVector.h>
#include <co/Log.h>

#include <sstream>

#define NUM_SERVERS 7

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
    node->start( "ipc:///tmp/client.pipe", "ipc:///tmp/client.pipe" );
    
    CORAL_LOG(INFO) << "Client Node started";
    
    co::RefVector<rpcTests::ISimpleTypes> controlVec;
    std::vector<std::string> endpointVec;
    
    // get the endpoints for all the servers
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        std::stringstream ss( std::stringstream::in | std::stringstream::out );
        ss << "ipc:///tmp/server" << i << ".pipe";
        endpointVec.push_back( ss.str() );
    }
    
    // Get the control interfaces of every server
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        co::IObject* controlObj = node->findRemoteInstance( "rpcTests.TestComponent", "control", endpointVec[i] );
        controlVec.push_back( controlObj->getService<rpcTests::ISimpleTypes>() );
    }
    
    //---------------------------------------//
    // Begin the test
    //---------------------------------------//
    
    // simple get and set test
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        controlVec[i]->setDouble( 0.1 );
        EXPECT_EQ( controlVec[i]->getStoredDouble(), 0.1 );
    }
    
    // get the incrementers
    co::RefVector<rpcTests::IIncrementer> incrVec;
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        co::IObject* incrObj = node->findRemoteInstance( "rpcTests.Incrementer", "incrementer", endpointVec[i] );
        incrVec.push_back( incrObj->getService<rpcTests::IIncrementer>() );
    }
    
    // test incrementation without barrier (should increment)
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        EXPECT_EQ( incrVec[i]->getNumberOne(), 1 );
        incrVec[i]->incrementAsync();
        EXPECT_EQ( incrVec[i]->getNumberOne(), 2 );
    }
    
    // test incrementation with barrier (should not increment)
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        EXPECT_EQ( incrVec[i]->getNumberOne(), 2 );
        incrVec[i]->incrementSync();
        EXPECT_EQ( incrVec[i]->getNumberOne(), 2 );
    }
    node->raiseBarrier( NUM_SERVERS );
    
    // receive the "barrier hit" messages
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        node->update();
    }
    
    // check if the values are incremented
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        EXPECT_EQ( incrVec[i]->getNumberOne(), 3 );
    }
    
    //shutting down the servers
    for( int i = 0; i < NUM_SERVERS; i++ )
    {
        controlVec[i]->setStoredInt( 1 );
    }
    
    node->stop();
}
