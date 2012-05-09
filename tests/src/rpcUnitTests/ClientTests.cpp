
#include <gtest/gtest.h>

#include "Node.h"
#include <ClientProxy.h>

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

    
typedef Unmarshaller::MsgType MsgType;
    
TEST( ClientTests, valueTypeCalls )
{
    co::RefPtr<co::IObject> fakeLinkObj = co::newInstance( "mockReef.FakeLink" );
    mockReef::IFakeLink* fakeLink = fakeLinkObj->getService<mockReef::IFakeLink>();
    rpc::IActiveLink* activeLink = fakeLinkObj->getService<rpc::IActiveLink>();
    
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
	co::IMethod* setIntMethod = co::cast<co::IMethod>( STInterface->getMember( "setInt" ) );
    
    STService->setInt( 3 );
    std::string msg;
    fakeLink->getMsg( msg );
    
    // parameters common to message types
    MsgType msgType;
    bool hasReturn;
    co::int32 msgReceiverID;
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    
    EXPECT_EQ( msgType, Unmarshaller::CALL );
    EXPECT_EQ( msgReceiverID, 9 );
    EXPECT_FALSE( hasReturn );
    
    // ------ call value types TODO:Complex types
    co::int32 facetIdx;
    co::int32 memberIdx;
    
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( memberIdx, setIntMethod->getIndex() );
    
    co::Any intParam;
    unmarshaller.unmarshalValueParam( intParam, co::getType( "int32" ) );
    EXPECT_EQ( intParam.get<co::int32>(), 3 );
    
    // Test the reply
    intParam.set<co::int32>( 4 );
    marshaller.marshalData( intParam, msg );
    fakeLink->setReply( msg );
    
    EXPECT_EQ( STService->getStoredInt(), 4 );
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
    Unmarshaller::RefOwner refOwner;
    std::string instanceType;
    std::string ownerAddress;
    
    // ------ Transmission of a Local Object ------ //
    marshaller.marshalData( intParam, msg );
    fakeLinkA->setReply( msg );
    EXPECT_EQ( refTypes->callIncrementInt( simpleTypesLocal, 1 ), 5 );
    fakeLinkA->getMsg( msg );

    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    EXPECT_EQ( msgType, Unmarshaller::CALL );
    EXPECT_EQ( msgReceiverID, 3 );
    EXPECT_TRUE( hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    unmarshaller.unmarshalRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 1 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::LOCAL );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressLocal" );
    
    // ------ Transmission of an Object belonging to the receiver ------ //
    ownerAddress = "notUsedWhenReceiverIsOwner";
    marshaller.marshalData( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesA2, 1 );
    fakeLinkA->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    unmarshaller.unmarshalRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::RECEIVER );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "notUsedWhenReceiverIsOwner" );
    
    // ------ Transmission of an Object belonging to another owner ------ //
    marshaller.marshalData( intParam, msg );
    fakeLinkA->setReply( msg );
    refTypes->callIncrementInt( simpleTypesB, 1 );
    fakeLinkA->getMsg( msg );
    
    unmarshaller.setMarshalledRequest( msg, msgType, msgReceiverID, hasReturn );
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx );
    EXPECT_EQ( facetIdx, RTPort->getIndex() );
    EXPECT_EQ( memberIdx, callIncrIntMethod->getIndex() );
    
    unmarshaller.unmarshalRefParam( instanceID, facetIdx, refOwner, instanceType, ownerAddress );
    EXPECT_EQ( instanceID, 3 );
    EXPECT_EQ( facetIdx, STPort->getIndex() );
    EXPECT_EQ( refOwner, Unmarshaller::ANOTHER );
    EXPECT_STREQ( instanceType.c_str(), "moduleA.TestComponent" );
    EXPECT_STREQ( ownerAddress.c_str(), "addressB" );
}
    
}
    
}