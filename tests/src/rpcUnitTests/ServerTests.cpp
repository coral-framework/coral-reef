#include <gtest/gtest.h>

#include "Node.h"
#include "Marshaller.h"
#include "Unmarshaller.h"
#include <ClientProxy.h>
#include <Message.pb.h>
#include "Invoker.h"

#include <moduleA/ISimpleTypes.h>
#include <moduleA/IReferenceTypes.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

#include <iostream>

namespace reef {
namespace rpc {
    
TEST( ServerTests, invokerValueTypeTest )
{
    Unmarshaller unmarshaller;
    Marshaller marshaller;

    // create remote object of TestComponent type
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "moduleA.TestComponent" ) );
    co::RefPtr<co::IObject> TCObject = co::newInstance( "moduleA.TestComponent" );
    
	// get the ports so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
    
	// get the ISimpleTypes service of TestComponent
	moduleA::ISimpleTypes* STService = TCObject->getService<moduleA::ISimpleTypes>();
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STService->getInterface();
    co::IInterface* parent = STInterface->getBaseType();
    co::IInterface* gParent = parent->getBaseType();
	co::IMethod* incrIntMethod = co::cast<co::IMethod>( STInterface->getMember( "incrementInt" ) );
    co::IMethod* parentMultiply = co::cast<co::IMethod>( parent->getMember( "parentMultiply" ) );
    co::IField* gParentIntField = co::cast<co::IField>( gParent->getMember( "grandParentInt" ) );
    
    Invoker invoker( 0, TCObject.get() );

    co::Any intParam; intParam.set<co::int32>( 4 );
    std::string msg;
    std::string caller = "irrelevant";
    
	// ------ Simple value types ------ //
    marshaller.beginCallMarshalling( 1, STPort->getIndex(), incrIntMethod->getIndex(), -1, true, caller );
    marshaller.addValueParam( intParam );
    marshaller.getMarshalledCall( msg );
    
    Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;

    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    invoker.invoke( unmarshaller, true, msg );
    unmarshaller.unmarshalValue( msg, co::getType( "int32" ), intParam );
    EXPECT_EQ( intParam.get<co::int32>(), 5 );
    
    // ------ Inherited Simple value types ------ //
    marshaller.beginCallMarshalling( 1, STPort->getIndex(), parentMultiply->getIndex(), 0, true, caller );
    co::Any doubleParam; doubleParam.set<double>( 5 );
    intParam.set<co::int32>( 2 );
    marshaller.addValueParam( doubleParam );
    marshaller.addValueParam( intParam );
    marshaller.getMarshalledCall( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    invoker.invoke( unmarshaller, true, msg );
    unmarshaller.unmarshalValue( msg, co::getType( "int32" ), intParam );
    EXPECT_EQ( intParam.get<co::int32>(), 10 );
        
    // ------ Inherited from grandparent Simple value types ------ //
    STService->setGrandParentInt( 2 );
    marshaller.beginCallMarshalling( 1, STPort->getIndex(), gParentIntField->getIndex(), 1, true, caller );
    marshaller.getMarshalledCall( msg );
    intParam.set<co::int32>( 2 );
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    invoker.invoke( unmarshaller, true, msg );
    unmarshaller.unmarshalValue( msg, co::getType( "int32" ), intParam );
    EXPECT_EQ( intParam.get<co::int32>(), 2 );
}

TEST( ServerTests, invokerReceivesRefTypeTests )
{
	Unmarshaller unmarshaller;
    Marshaller marshaller;

	// Node is needed internally
    co::RefPtr<Node> node = new rpc::Node();
    rpc::ITransport* transport = co::newInstance( "mockReef.Transport" )->getService<rpc::ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );

	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "moduleA.TestComponent" ) );

	// get the ports so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );
	co::IPort* RTPort = co::cast<co::IPort>( TCComponent->getMember( "reference" ) );

	co::IInterface* RTInterface = RTPort->getType();
    co::IInterface* parent = RTInterface->getBaseType();
	co::IField* simpleField = co::cast<co::IField>( RTInterface->getMember( "simple" ) );
	co::IField* parentSimple = co::cast<co::IField>( parent->getMember( "parentSimple" ) );

	// Creates 2 instances, one for publishing and other for setting as the invoker's instance
	co::RefPtr<co::IObject> TCObj = co::newInstance( "moduleA.TestComponent" );
	co::RefPtr<co::IObject> TCObj2 = co::newInstance( "moduleA.TestComponent" );
	node->publishInstance( TCObj2.get(), "irrelevant" );

	// Create the invoker
	Invoker invoker( node.get(), TCObj.get() );
    
    std::string caller = "irrelevant";

	// ------ Receiving and returning reference to a local object ------ //
	// Receiving //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), -1, false,
                                    caller );
	marshaller.addReferenceParam( 1, STPort->getIndex(), Marshaller::RECEIVER );
	std::string request;
    marshaller.getMarshalledCall( request );

	Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	invoker.invoke( unmarshaller, false, request );

	moduleA::IReferenceTypes* reference = TCObj->getService<moduleA::IReferenceTypes>();
	EXPECT_EQ( TCObj2->getService<moduleA::ISimpleTypes>(), reference->getSimple() );

	// Returning //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), -1, true, 
                                    caller );
    marshaller.getMarshalledCall( request );

	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	std::string returned;
	invoker.invoke( unmarshaller, true, returned );

	co::int32 returnedinstanceId;
	co::int32 facetIdx;
	Unmarshaller::RefOwner refOwner;
	std::string instanceType;
	std::string ownerAddress;
	unmarshaller.unmarshalReference( returned, returnedinstanceId, facetIdx, refOwner, instanceType, ownerAddress );

	EXPECT_EQ( returnedinstanceId, 1 );
	EXPECT_EQ( facetIdx, STPort->getIndex() );
	EXPECT_EQ( refOwner, Unmarshaller::LOCAL );
	EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
	EXPECT_STREQ( ownerAddress.c_str(), "addressLocal" );

	// ------ Receiving and returning reference to a remote object ------ //
	// Receiving: A call setParentSimple invocation will be simulated. This is an inherited field.
    
    // irrelevant, ReferenceTypes Port index, parentSimple field index, parent depth (0), not synch
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), parentSimple->getIndex(), 0, false, 
                                    caller );

	instanceType = "moduleA.TestComponent";
	ownerAddress = "addressRemote";
    // The parameter is a reference to ANOTHER, the facet is ISimpleTypes
	marshaller.addReferenceParam( 2, STPort->getIndex(), Marshaller::ANOTHER, &instanceType, &ownerAddress );

    marshaller.getMarshalledCall( request );

    // Sets the unmarshaller to the correct state to be passed to the invoker.
	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
    /* The invoker will set the field parentSimple in its controlled object. For that, a new 
     ClientProxy to addressRemote's instance 2 will be created. */
	invoker.invoke( unmarshaller, false, request );

    // Now checks if the ClientProxy was created correctly
	co::RefPtr<moduleA::ISimpleTypes> simple = reference->getParentSimple();
	ClientProxy* providerCP = static_cast<ClientProxy*>( simple->getProvider() );
    EXPECT_EQ( providerCP->getInstanceId(), 2 );
	EXPECT_STREQ( providerCP->getOwnerAddress().c_str(), "addressRemote" );

	// Returning reference to local object
	// Returning //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), parentSimple->getIndex(), 0, true, 
                                    caller );
    marshaller.getMarshalledCall( request );

	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	invoker.invoke( unmarshaller, true, returned );

	returnedinstanceId = -1;
	facetIdx = -1;
	instanceType = "";
	unmarshaller.unmarshalReference( returned, returnedinstanceId, facetIdx, refOwner, instanceType, ownerAddress );

	EXPECT_EQ( returnedinstanceId, 2 );
	EXPECT_EQ( facetIdx, STPort->getIndex() );
	EXPECT_EQ( refOwner, Unmarshaller::ANOTHER );
	EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
	EXPECT_STREQ( ownerAddress.c_str(), "addressRemote" );
	
}

TEST( ServerTests, nodeTest )
{
    // Node is needed by the ClientProxys to publish the local instances
    co::RefPtr<Node> node = new rpc::Node();
    ITransport* transport = co::newInstance( "mockReef.Transport" )->getService<ITransport>();
    node->setService( "transport", transport );
    node->start( "addressLocal", "addressLocal" );
    co::RefPtr<co::IObject> TCObject1 = co::newInstance( "moduleA.TestComponent" );
    co::RefPtr<co::IObject> TCObject2 = co::newInstance( "moduleA.TestComponent" );
    co::RefPtr<co::IObject> TCObject3 = co::newInstance( "moduleA.TestComponent" );
    
    std::string caller = "irrelevant";
    
    co::int32 instanceId = node->publishAnonymousInstance( TCObject1.get(), caller );
    EXPECT_EQ( instanceId, 1 );
    instanceId = 2;
    instanceId = node->publishAnonymousInstance( TCObject1.get(), caller );
    EXPECT_EQ( instanceId, 1 );
    instanceId = node->publishAnonymousInstance( TCObject2.get(), caller );
    EXPECT_EQ( instanceId, 2 );
    instanceId = node->publishAnonymousInstance( TCObject3.get(), caller );
    EXPECT_EQ( instanceId, 3 );
    
    co::RefPtr<co::IObject> object = node->getInstance( 1 );
    EXPECT_TRUE( object.get() == TCObject1.get() );
    
    object = node->getRemoteInstance( "moduleA.TestComponent", 4, "address" );    
}
    
}
            
}