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
	co::IPort* RTPort = co::cast<co::IPort>( TCComponent->getMember( "reference" ) );
    
	// get the ISimpleTypes service of TestComponent
	moduleA::ISimpleTypes* STService = TCObject->getService<moduleA::ISimpleTypes>();
    
    // get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
    co::IInterface* STInterface = STService->getInterface();
	co::IMethod* incrIntMethod = co::cast<co::IMethod>( STInterface->getMember( "incrementInt" ) );

    Invoker invoker( 0, TCObject.get() );

    co::Any intParam; intParam.set<co::int32>( 4 );
    std::string msg;
    
	// ------ Simple value types ------ //
    marshaller.beginCallMarshalling( 1, STPort->getIndex(), incrIntMethod->getIndex(), true );
    marshaller.addValueParam( intParam );
    marshaller.getMarshalledCall( msg );
    
    Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;

    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    invoker.synchCall( unmarshaller, msg );
    unmarshaller.unmarshalValue( msg, co::getType( "int32" ), intParam );
    EXPECT_EQ( intParam.get<co::int32>(), 5 );
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
	co::IField* simpleField = co::cast<co::IField>( RTInterface->getMember( "simple" ) );

	// Creates 2 instances, one for publishing and other for setting as the invoker's instance
	co::RefPtr<co::IObject> TCObj = co::newInstance( "moduleA.TestComponent" );
	co::RefPtr<co::IObject> TCObj2 = co::newInstance( "moduleA.TestComponent" );
	node->publishInstance( TCObj2.get(), "irrelevant" );

	// Create the invoker
	Invoker invoker( node.get(), TCObj.get() );

	// ------ Receiving and returning reference to a local object ------ //
	// Receiving //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), false );
	marshaller.addReferenceParam( 1, STPort->getIndex(), Marshaller::RECEIVER );
	std::string request;
    marshaller.getMarshalledCall( request );

	Unmarshaller::MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	invoker.asynchCall( unmarshaller );

	moduleA::IReferenceTypes* reference = TCObj->getService<moduleA::IReferenceTypes>();
	EXPECT_EQ( TCObj2->getService<moduleA::ISimpleTypes>(), reference->getSimple() );

	// Returning //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), true );
    marshaller.getMarshalledCall( request );

	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	std::string returned;
	invoker.synchCall( unmarshaller, returned );

	co::int32 returnedInstanceID;
	co::int32 facetIdx;
	Unmarshaller::RefOwner refOwner;
	std::string instanceType;
	std::string ownerAddress;
	unmarshaller.unmarshalReference( returned, returnedInstanceID, facetIdx, refOwner, instanceType, ownerAddress );

	EXPECT_EQ( returnedInstanceID, 1 );
	EXPECT_EQ( facetIdx, STPort->getIndex() );
	EXPECT_EQ( refOwner, Unmarshaller::LOCAL );
	EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
	EXPECT_STREQ( ownerAddress.c_str(), "addressLocal" );

	// ------ Receiving and returning reference to a remote object ------ //
	// Receiving //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), false );

	instanceType = "moduleA.TestComponent";
	ownerAddress = "addressRemote";
	marshaller.addReferenceParam( 2, STPort->getIndex(), Marshaller::ANOTHER, &instanceType, &ownerAddress );

    marshaller.getMarshalledCall( request );

	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	invoker.asynchCall( unmarshaller );

	co::RefPtr<moduleA::ISimpleTypes> simple = reference->getSimple();
	ClientProxy* providerRO = static_cast<ClientProxy*>( simple->getProvider() );
    IInstanceInfo* info = static_cast<IInstanceInfo*>( providerRO );
        
    EXPECT_EQ( info->getInstanceID(), 2 );
	EXPECT_STREQ( info->getOwnerAddress().c_str(), "addressRemote" );

	// Returning reference to local object
	// Returning //
	marshaller.beginCallMarshalling( 1, RTPort->getIndex(), simpleField->getIndex(), true );
    marshaller.getMarshalledCall( request );

	unmarshaller.setMarshalledRequest( request, msgType, msgReceiverID, hasReturn );
	invoker.synchCall( unmarshaller, returned );

	returnedInstanceID = -1;
	facetIdx = -1;
	instanceType = "";
	unmarshaller.unmarshalReference( returned, returnedInstanceID, facetIdx, refOwner, instanceType, ownerAddress );

	EXPECT_EQ( returnedInstanceID, 2 );
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
    
    co::int32 instanceID = node->publishAnonymousInstance( TCObject1.get() );
    EXPECT_EQ( instanceID, 1 );
    instanceID = 2;
    instanceID = node->publishAnonymousInstance( TCObject1.get() );
    EXPECT_EQ( instanceID, 1 );
    instanceID = node->publishAnonymousInstance( TCObject2.get() );
    EXPECT_EQ( instanceID, 2 );
    instanceID = node->publishAnonymousInstance( TCObject3.get() );
    EXPECT_EQ( instanceID, 3 );
    
    co::RefPtr<co::IObject> object = node->getInstance( 1 );
    EXPECT_TRUE( object.get() == TCObject1.get() );
    
    object = node->getRemoteInstance( "moduleA.TestComponent", 4, "address" );    
}
    
}
            
}