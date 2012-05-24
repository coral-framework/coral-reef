
#include <gtest/gtest.h>

#include "Node.h"
#include "ClientProxy.h"

#include <mockReef/IFakeLink.h>
#include <moduleA/ISimpleTypes.h>
#include <moduleA/IReferenceTypes.h>
#include <reef/rpc/ITransport.h>
#include <reef/rpc/IActiveLink.h>
#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

namespace reef {
namespace rpc {

    
TEST( ClientTests, valueTypeCalls )
{
    co::RefPtr<co::IObject> fakeLinkObj = co::newInstance( "mockReef.FakeLink" );
    mockReef::IFakeLink* fakeLink = fakeLinkObj->getService<mockReef::IFakeLink>();
    rpc::IActiveLink* activeLink = fakeLinkObj->getService<rpc::IActiveLink>();
    fakeLink->setAddress( "address" );
    
    Unmarshaller unmarshaller;
    Marshaller marshaller;
    
    // Node is needed internally
    co::RefPtr<Node> node = new rpc::Node();
    rpc::ITransport* transport = co::newInstance( "mockReef.Transport" )->getService<rpc::ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );

    // create remote object of TestComponent type
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "moduleA.TestComponent" ) );
    co::RefPtr<ClientProxy> TCObject = ClientProxy::getOrCreateClientProxy( node.get(), TCComponent, activeLink,
                                                                             9 );

	// get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );

	// get the ISimpleTypes service of TestComponent
	moduleA::ISimpleTypes* STService = TCObject->getService<moduleA::ISimpleTypes>();
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STService->getInterface();
    co::IInterface* parent = STInterface->getBaseType();
    co::IInterface* gParent = parent->getBaseType();
    
    EXPECT_EQ( parent, STInterface->getSuperTypes()[0] );
    EXPECT_EQ( gParent, STInterface->getSuperTypes()[1] );
    
    // If this assert fails, coral's internal type hierarchy has changed.
    ASSERT_EQ( 3, STInterface->getSuperTypes().getSize() );
    
	co::IMethod* setIntMethod = co::cast<co::IMethod>( STInterface->getMember( "setInt" ) );
    co::IField* parentIntField = co::cast<co::IField>( parent->getMember( "parentInt" ) );
    co::IField* gParentIntField = co::cast<co::IField>( gParent->getMember( "grandParentInt" ) );
    
    // Testing a regular method //
    STService->setInt( 3 );
    std::string msg;
    fakeLink->getMsg( msg );
    
    // parameters common to message types
    Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_EQ( msgType, Unmarshaller::CALL );
    EXPECT_EQ( msgReceiverID, 9 );
    EXPECT_FALSE( hasReturn );
    
    // ------ call value types TODO:Complex types
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::int32 memberOwner;
    std::string caller;
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, memberOwner, caller );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( memberIdx, setIntMethod->getIndex() );
    EXPECT_EQ( memberOwner, -1 );
    
    co::Any intParam;
    unmarshaller.unmarshalValueParam( intParam, co::getType( "int32" ) );
    EXPECT_EQ( intParam.get<co::int32>(), 3 );
    
    // Test the reply
    intParam.set<co::int32>( 4 );
    marshaller.marshalValueType( intParam, msg );
    fakeLink->setReply( msg );
    
    EXPECT_EQ( STService->getStoredInt(), 4 );
    
    // Testing an inherited method //
    STService->setParentInt( 4 );
    fakeLink->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_FALSE( hasReturn );
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, memberOwner, caller );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( memberIdx, parentIntField->getIndex() );
    EXPECT_EQ( memberOwner, 0 );
    
    unmarshaller.unmarshalValueParam( intParam, co::getType( "int32" ) );
    EXPECT_EQ( intParam.get<co::int32>(), 4 );
    
    // Test the reply
    intParam.set<co::int32>( 5 );
    marshaller.marshalValueType( intParam, msg );
    fakeLink->setReply( msg );
    
    EXPECT_EQ( STService->getParentInt(), 5 );
    
    // Testing a method inherited from grandparent //
    STService->setGrandParentInt( 5 );
    fakeLink->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_FALSE( hasReturn );
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, memberOwner, caller );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( memberIdx, gParentIntField->getIndex() );
    EXPECT_EQ( memberOwner, 1 );
    
    unmarshaller.unmarshalValueParam( intParam, co::getType( "int32" ) );
    EXPECT_EQ( intParam.get<co::int32>(), 5 );
    
    // Test the reply
    intParam.set<co::int32>( 6 );
    marshaller.marshalValueType( intParam, msg );
    fakeLink->setReply( msg );
    
    EXPECT_EQ( STService->getGrandParentInt(), 6 );
}
    
TEST( ClientTests, refTypeCalls )
{
    Unmarshaller unmarshaller;
    Marshaller marshaller;
    
    /* ------ Initialization of remote objects for Testing all cases of ref type parameter
       ------(refer to reef's wiki for the cases of ref type params)               ------ */
    co::RefPtr<co::IObject> fakeLinkObjA = co::newInstance( "mockReef.FakeLink" );
    mockReef::IFakeLink* fakeLinkA = fakeLinkObjA->getService<mockReef::IFakeLink>();
    rpc::IActiveLink* activeLinkA = fakeLinkObjA->getService<rpc::IActiveLink>();
    fakeLinkA->setAddress( "addressA" );
    
    co::RefPtr<co::IObject> fakeLinkObjB = co::newInstance( "mockReef.FakeLink" );
    rpc::IActiveLink* activeLinkB = fakeLinkObjB->getService<rpc::IActiveLink>();
    mockReef::IFakeLink* fakeLinkB = fakeLinkObjB->getService<mockReef::IFakeLink>();
    fakeLinkB->setAddress( "addressB" );
   
    // Node is needed internally
    co::RefPtr<Node> node = new rpc::Node();
    rpc::ITransport* transport = co::newInstance( "mockReef.Transport" )->getService<rpc::ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );
    
    co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "moduleA.TestComponent" ) );
    
    // A supposedly remote object for a TC in host "A"
    co::RefPtr<co::IObject> remoteObjectA1 = ClientProxy::getOrCreateClientProxy( 
                                                            node.get(), TCComponent, activeLinkA, 3 );
    // A supposedly remote object for another TC in host "A"
    co::RefPtr<co::IObject> remoteObjectA2 = ClientProxy::getOrCreateClientProxy( 
                                                            node.get(), TCComponent, activeLinkA, 3 );
    moduleA::ISimpleTypes* simpleTypesA2 = remoteObjectA2->getService<moduleA::ISimpleTypes>();
    
    // A supposedly remote object for a TC in host "B"
    co::RefPtr<co::IObject> remoteObjectB = ClientProxy::getOrCreateClientProxy( 
                                                            node.get(), TCComponent, activeLinkB, 3 );
    moduleA::ISimpleTypes* simpleTypesB = remoteObjectB->getService<moduleA::ISimpleTypes>();
    
    // A local TC
    co::RefPtr<co::IObject> localObject = co::newInstance( "moduleA.TestComponent" );
    moduleA::ISimpleTypes* simpleTypesLocal = localObject->getService<moduleA::ISimpleTypes>();
    
    moduleA::IReferenceTypes* refTypes = remoteObjectA1->getService<moduleA::IReferenceTypes>();
    
    // get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
    co::IPort* RTPort = co::cast<co::IPort>( TCComponent->getMember( "reference" ) );
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* RTInterface = RTPort->getType();
    co::IInterface* parent = RTInterface->getBaseType();
	co::IMethod* callIncrIntMethod = co::cast<co::IMethod>( RTInterface->getMember( "callIncrementInt" ) );
    co::IMethod* parentCall = co::cast<co::IMethod>( parent->getMember( "parentCall" ) );
    
    // ------ test the transmission of objects ------ //
    
    // All the necessary variables for the ref type transmission tests
    co::Any intParam; intParam.set<co::int32>( 5 );
    std::string msg;    
    Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::int32 instanceId;
    co::int32 typeDepth;
    Unmarshaller::RefOwner refOwner;
    std::string instanceType;
    std::string ownerAddress;
    std::string caller;
    
    // ------ Transmission of a Local Object ------ //
    marshaller.marshalValueType( intParam, msg );
    fakeLinkA->setReply( msg );
    EXPECT_EQ( refTypes->parentCall( simpleTypesLocal, 1 ), 5 );
    fakeLinkA->getMsg( msg );

    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    EXPECT_EQ( msgType, Unmarshaller::CALL );
    EXPECT_EQ( msgReceiverID, 3 );
    EXPECT_TRUE( hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, typeDepth, caller );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, parentCall->getIndex() );
    EXPECT_EQ( typeDepth, 0 );
    
    unmarshaller.unmarshalReferenceParam( instanceId, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceId, 1 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::LOCAL );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressLocal" );
    
    // ------ Transmission of an Object belonging to the receiver ------ //
    ownerAddress = "notUsedWhenReceiverIsOwner";
    marshaller.marshalValueType( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesA2, 1 );
    fakeLinkA->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, typeDepth, caller );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    unmarshaller.unmarshalReferenceParam( instanceId, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceId, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::RECEIVER );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "notUsedWhenReceiverIsOwner" );
    
    // ------ Transmission of an Object belonging to another owner ------ //
    marshaller.marshalValueType( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesB, 1 );
    fakeLinkA->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx, typeDepth, caller );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    unmarshaller.unmarshalReferenceParam( instanceId, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceId, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::ANOTHER );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressB" );
}

TEST( ClientTests, refTypeReturns )
{
	Unmarshaller unmarshaller;
    Marshaller marshaller;

	/* ------ Initialization of remote objects for Testing all cases of ref type parameter
       ------(refer to reef's wiki for the cases of ref type params)               ------ */
    co::RefPtr<co::IObject> fakeLinkObjA = co::newInstance( "mockReef.FakeLink" );
    mockReef::IFakeLink* fakeLinkA = fakeLinkObjA->getService<mockReef::IFakeLink>();
    rpc::IActiveLink* activeLinkA = fakeLinkObjA->getService<rpc::IActiveLink>();
    fakeLinkA->setAddress( "addressA" );
   
    // Node is needed internally
    co::RefPtr<Node> node = new rpc::Node();
    rpc::ITransport* transport = co::newInstance( "mockReef.Transport" )->getService<rpc::ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );
    
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "moduleA.TestComponent" ) );
	
	// get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
    
    // A supposedly remote object for a TC in host "A"
    co::RefPtr<co::IObject> remoteObjectA = ClientProxy::getOrCreateClientProxy( 
                                                            node.get(), TCComponent, activeLinkA, 3 );
   
	moduleA::IReferenceTypes* refTypes = remoteObjectA->getService<moduleA::IReferenceTypes>();

	// Creating a local object that will be used internally when its id is returned
	co::RefPtr<co::IObject> localObj = co::newInstance( "moduleA.TestComponent" );
	// Publishing the local object so when its id is returned, the CLientProxy can access it
	node->publishInstance( localObj.get(), "irrelevant" );
	moduleA::ISimpleTypes* localSimple = localObj->getService<moduleA::ISimpleTypes>();
	localSimple->setStoredInt( 5 );

	// Incept the local object's id as a to-be-returned value into the fakelink 
	std::string reference;
	marshaller.marshalReferenceType( 1, STPort->getIndex(), Marshaller::RECEIVER, reference );
	fakeLinkA->setReply( reference );

	// Call a method that will receive the incepted local obj id as return value
	EXPECT_EQ( refTypes->getSimple()->getStoredInt(), 5 );

	std::string instanceType = "moduleA.TestComponent";
	std::string ownerAddress = "address";
	marshaller.marshalReferenceType( 5, STPort->getIndex(), Marshaller::ANOTHER, reference,
								&instanceType, &ownerAddress );
	fakeLinkA->setReply( reference );

	// Call a method that will receive the incepted local obj id as return value
	co::RefPtr<moduleA::ISimpleTypes> simple = refTypes->getParentSimple();
	ClientProxy* providerCP = static_cast<ClientProxy*>( simple->getProvider() );
        
     EXPECT_EQ( providerCP->getInstanceId(), 5 );
	 EXPECT_STREQ( providerCP->getOwnerAddress().c_str(), "address" );

	 node->unpublishInstance( "irrelevant" );
}

}
    
}