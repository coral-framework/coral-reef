
#include <gtest/gtest.h>

#include <testModule/MotherStruct.h>
#include <testModule/ISimpleTypes.h>
#include <testModule/IComplexTypes.h>

#include <RemoteObject.h>
#include "RemObjTestChannel.h"

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

namespace reef
{

TEST( ClientTests, remoteObjectTest )
{
	// fake channel to receive calls from the remote object
	RemObjTestChannel* fakeChannel = new RemObjTestChannel();

	// create remote object of TestComponent type
	co::IComponent* TCComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
	co::IObject* TCObject = new RemoteObject( TCComponent, fakeChannel  );

	// get the ISimpleTypes port so we can know the index of the port to check later.
	co::IPort* STPort = co::cast<co::IPort>( TCComponent->getMember( "simple" ) );

	// get the ISimpleTypes service of TestComponent
	testModule::ISimpleTypes* STService = co::cast<testModule::ISimpleTypes>( TCObject->getServiceAt( STPort ) );

	// get the ISimpleTypes service's interface. Then, get the methods we want to check indices later.
	co::IInterface* STInterface = STService->getInterface();
	co::IMethod* setIntMethod = co::cast<co::IMethod>( STInterface->getMember( "setInt" ) );
	co::IMethod* setStringMethod = co::cast<co::IMethod>( STInterface->getMember( "setString" ) );
	co::IField* storedIntField = co::cast<co::IField>( STInterface->getMember( "storedInt" ) );
	co::IField* storedStringListField = co::cast<co::IField>( STInterface->getMember( "storedStringList" ) );
	co::IField* storedStringField = co::cast<co::IField>( STInterface->getMember( "storedString" ) );

	// Now start calling methods and set/getting fields to test
	STService->setInt( 1 );
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), setIntMethod, 0 ) );
	STService->setString( std::string( "" ) );
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), setStringMethod, 0 ) );
	STService->setStoredInt( 1 );
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), 0, storedIntField ) );

	std::vector<std::string> arg;
	STService->setStoredStringList( arg );
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), 0, storedStringListField ) );
	STService->getStoredInt();
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), 0, storedIntField ) );
	STService->getStoredString();
	EXPECT_TRUE( fakeChannel->compareCalledValues( STPort->getIndex(), 0, storedStringField ) );


	// get the IComplexTypes port so we can know the index of the port to check later.
	co::IPort* CTPort = co::cast<co::IPort>( TCComponent->getMember( "complex" ) );
	testModule::IComplexTypes* CTService = co::cast<testModule::IComplexTypes>( TCObject->getServiceAt( CTPort ) );
	co::IInterface* CTInterface = CTService->getInterface();
	co::IField* motherStructField = co::cast<co::IField>( CTInterface->getMember( "motherStruct" ) );

	testModule::MotherStruct ms;
	CTService->setMotherStruct( ms );
	EXPECT_TRUE( fakeChannel->compareCalledValues( CTPort->getIndex(), 0, motherStructField ) );
}

}