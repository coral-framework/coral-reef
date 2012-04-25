#include <gtest/gtest.h>

#include "Node.h"
#include "Encoder.h"
#include "Decoder.h"
#include <RemoteObject.h>
#include <Message.pb.h>
#include "Servant.h"

#include <testModule/ISimpleTypes.h>
#include <testModule/IComplexTypes.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

#include <iostream>

namespace reef
{

typedef Decoder::MsgType MsgType;
    
TEST( ServerTests, servantTest )
{
    Decoder decoder;
    Encoder encoder;
    
    // create remote object of TestComponent type
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    co::RefPtr<co::IObject> TCObject = co::newInstance( "testModule.TestComponent" );
    
	// get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
    
	// get the ISimpleTypes service of TestComponent
	testModule::ISimpleTypes* STService = TCObject->getService<testModule::ISimpleTypes>();
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STService->getInterface();
	co::IMethod* incrIntMethod = co::cast<co::IMethod>( STInterface->getMember( "incrementInt" ) );

    Servant servant( 0, TCObject.get() );

    co::Any intParam; intParam.set<co::int32>( 4 );
    std::string msg;
    
    encoder.beginEncodingCallMsg( 1, STPort->getIndex(), incrIntMethod->getIndex(), false );
    encoder.addValueParam( intParam );
    encoder.finishEncodingCallMsg( msg );
    
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;

    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    servant.onCallOrField( decoder, &intParam );
    EXPECT_EQ( intParam.get<co::int32>(), 5 );
    
}
    
TEST( ServerTests, nodeTest )
{
    // Node is needed by the RemoteObjects to publish the local instances
    Node* node = new reef::Node();
    ITransport* transport = co::newInstance( "testTransport.Transport" )->getService<ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );
    co::RefPtr<co::IObject> TCObject1 = co::newInstance( "testModule.TestComponent" );
    co::RefPtr<co::IObject> TCObject2 = co::newInstance( "testModule.TestComponent" );
    co::RefPtr<co::IObject> TCObject3 = co::newInstance( "testModule.TestComponent" );
    
    co::int32 instanceID = node->publishInstance( TCObject1.get() );
    EXPECT_EQ( instanceID, 1 );
    instanceID = 2;
    instanceID = node->publishInstance( TCObject1.get() );
    EXPECT_EQ( instanceID, 1 );
    instanceID = node->publishInstance( TCObject2.get() );
    EXPECT_EQ( instanceID, 2 );
    instanceID = node->publishInstance( TCObject3.get() );
    EXPECT_EQ( instanceID, 3 );
    
    co::IObject* object = node->getInstanceFor( 1 );
    EXPECT_TRUE( object == TCObject1.get() );
    
    object = node->getRemoteInstance( "testModule.TestComponent", 4, "address" );    
}
        
}