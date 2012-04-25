
#include <gtest/gtest.h>

#include "Node.h"
#include <RemoteObject.h>

#include <testTransport/IFakeLink.h>
#include <testModule/ISimpleTypes.h>
#include <testModule/IReferenceTypes.h>
#include <reef/ITransport.h>
#include <reef/IActiveLink.h>
#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

namespace reef
{
    
typedef Decoder::MsgType MsgType;
    
TEST( ClientTests, valueTypeCalls )
{
    co::RefPtr<co::IObject> fakeLinkObj = co::newInstance( "testTransport.FakeLink" );
    testTransport::IFakeLink* fakeLink = fakeLinkObj->getService<testTransport::IFakeLink>();
    reef::IActiveLink* activeLink = fakeLinkObj->getService<reef::IActiveLink>();
    
    Decoder decoder;
    Encoder encoder;
    
    // create remote object of TestComponent type
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    co::RefPtr<RemoteObject> TCObject = RemoteObject::getOrCreateRemoteObject( 0, TCComponent, activeLink,
                                                                             9 );

	// get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );

	// get the ISimpleTypes service of TestComponent
	testModule::ISimpleTypes* STService = TCObject->getService<testModule::ISimpleTypes>();
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STService->getInterface();
	co::IMethod* setIntMethod = co::cast<co::IMethod>( STInterface->getMember( "setInt" ) );
    
    STService->setInt( 3 );
    std::string msg;
    fakeLink->getMsg( msg );
    
    // parameters common to message types
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_EQ( msgType, MsgType::CALL );
    EXPECT_EQ( msgReceiverID, 9 );
    EXPECT_FALSE( hasReturn );
    
    // ------ call value types TODO:Complex types
    co::int32 facetIdx;
    co::int32 memberIdx;
    
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( memberIdx, setIntMethod->getIndex() );
    
    co::Any intParam;
    decoder.getValueParam( intParam, co::getType( "int32" ) );
    EXPECT_EQ( intParam.get<co::int32>(), 3 );
    
    // Test the reply
    intParam.set<co::int32>( 4 );
    encoder.encodeData( intParam, msg );
    fakeLink->setReply( msg );
    
    EXPECT_EQ( STService->getStoredInt(), 4 );
}
    
TEST( ClientTests, refTypeCalls )
{
    Decoder decoder;
    Encoder encoder;
    
    /* ------ Initialization of remote objects for Testing all cases of ref type parameter
       ------(refer to reef's wiki for the cases of ref type params)               ------ */
    co::RefPtr<co::IObject> fakeLinkObjA = co::newInstance( "testTransport.FakeLink" );
    testTransport::IFakeLink* fakeLinkA = fakeLinkObjA->getService<testTransport::IFakeLink>();
    reef::IActiveLink* activeLinkA = fakeLinkObjA->getService<reef::IActiveLink>();
    fakeLinkA->setAddress( "addressA" );
    
    co::RefPtr<co::IObject> fakeLinkObjB = co::newInstance( "testTransport.FakeLink" );
    reef::IActiveLink* activeLinkB = fakeLinkObjB->getService<reef::IActiveLink>();
    testTransport::IFakeLink* fakeLinkB = fakeLinkObjB->getService<testTransport::IFakeLink>();
    fakeLinkB->setAddress( "addressB" );
   
    // Node is needed by the RemoteObjects to publish the local instances
    co::RefPtr<Node> node = new reef::Node();
    reef::ITransport* transport = co::newInstance( "testTransport.Transport" )->getService<reef::ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );
    
    co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    
    // A supposedly remote object for a TC in host "A"
    co::RefPtr<co::IObject> remoteObjectA1 = RemoteObject::getOrCreateRemoteObject( 
                                                            node.get(), TCComponent, activeLinkA, 3 );
    // A supposedly remote object for another TC in host "A"
    co::RefPtr<co::IObject> remoteObjectA2 = RemoteObject::getOrCreateRemoteObject( 
                                                            node.get(), TCComponent, activeLinkA, 3 );
    testModule::ISimpleTypes* simpleTypesA2 = remoteObjectA2->getService<testModule::ISimpleTypes>();
    
    // A supposedly remote object for a TC in host "B"
    co::RefPtr<co::IObject> remoteObjectB = RemoteObject::getOrCreateRemoteObject( 
                                                            node.get(), TCComponent, activeLinkB, 3 );
    testModule::ISimpleTypes* simpleTypesB = remoteObjectB->getService<testModule::ISimpleTypes>();
    
    // A local TC
    co::RefPtr<co::IObject> localObject = co::newInstance( "testModule.TestComponent" );
    testModule::ISimpleTypes* simpleTypesLocal = localObject->getService<testModule::ISimpleTypes>();
    
    testModule::IReferenceTypes* refTypes = remoteObjectA1->getService<testModule::IReferenceTypes>();
    
    // get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
    co::IPort* RTPort = co::cast<co::IPort>( TCComponent->getMember( "reference" ) );
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STPort->getType();
    co::IInterface* RTInterface = RTPort->getType();
	co::IMethod* callIncrIntMethod = co::cast<co::IMethod>( RTInterface->getMember( "callIncrementInt" ) );
    
    // ------ test the transmission of a objects ------ //
    
    // All the necessary variables for the ref type transmission tests
    co::Any intParam; intParam.set<co::int32>( 5 );
    std::string msg;    
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::int32 instanceID;
    Decoder::RefOwner refOwner;
    std::string instanceType;
    std::string ownerAddress;
    
    // ------ Transmission of a Local Object ------ //
    encoder.encodeData( intParam, msg );
    fakeLinkA->setReply( msg );
    EXPECT_EQ( refTypes->callIncrementInt( simpleTypesLocal, 1 ), 5 );
    fakeLinkA->getMsg( msg );

    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    EXPECT_EQ( msgType, MsgType::CALL );
    EXPECT_EQ( msgReceiverID, 3 );
    EXPECT_TRUE( hasReturn );
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    decoder.getRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 1 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Decoder::RefOwner::LOCAL );
    EXPECT_STREQ( instanceType.c_str(), "testModule.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressLocal" );
    
    // ------ Transmission of an Object belonging to the receiver ------ //
    ownerAddress = "notUsedWhenReceiverIsOwner";
    encoder.encodeData( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesA2, 1 );
    fakeLinkA->getMsg( msg );
    
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    decoder.getRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Decoder::RefOwner::RECEIVER );
    EXPECT_STREQ( instanceType.c_str(), "testModule.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "notUsedWhenReceiverIsOwner" );
    
    // ------ Transmission of an Object belonging to another owner ------ //
    encoder.encodeData( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesB, 1 );
    fakeLinkA->getMsg( msg );
    
    decoder.setMsgForDecoding( msg, msgType, msgReceiverID, hasReturn );
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    decoder.getRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Decoder::RefOwner::ANOTHER );
    EXPECT_STREQ( instanceType.c_str(), "testModule.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressB" );
}

}