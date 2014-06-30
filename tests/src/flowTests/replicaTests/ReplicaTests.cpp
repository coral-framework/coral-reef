/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include <gtest/gtest.h>

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/RefPtr.h>
#include <co/Log.h>
#include <co/IComponent.h>
#include <co/IReflector.h>
#include <co/IService.h>
#include <co/IllegalStateException.h>
#include <co/IllegalArgumentException.h>

#include "CompanySpace.h"
#include "LocalSpaceObserver.h"

#include <dom/ICompany.h>
#include <dom/IEmployee.h>
#include <dom/IProduct.h>
#include <dom/IService.h>

#include <ca/ISpace.h>
#include <ca/IModel.h>
#include <ca/IUniverse.h>
#include <ca/IOException.h>
#include <ca/IObjectChanges.h>

#include <flow/ISpaceSubscriber.h>
#include <flow/ISpacePublisher.h>

class ReplicaTests : public CompanySpace
{
public:

	void SetUp()
	{
		CompanySpace::SetUp();
	}

	void TearDown()
	{
		CompanySpace::TearDown();
	}

	void checkInitialSpace( co::IObject* objRest )
	{
		dom::ICompany* company = objRest->getService<dom::ICompany>();
		ASSERT_TRUE( company != NULL );

		co::TSlice<dom::IEmployee*> employees = company->getEmployees();
		ASSERT_EQ( 5, employees.getSize() );
	
		EXPECT_EQ( "Joseph Java Newbie", employees[0]->getName() );
		EXPECT_EQ( 1000, employees[0]->getSalary() );
		EXPECT_EQ( "Developer", employees[0]->getRole() );
		EXPECT_EQ( NULL, employees[0]->getLeading() );

		ASSERT_EQ( 1, employees[0]->getWorking().getSize() );
		dom::IProduct* devProduct;
		ASSERT_NO_THROW( devProduct = co::cast<dom::IProduct>( employees[0]->getWorking()[0] ) );
		EXPECT_EQ( "Software2.0", devProduct->getName() );
		EXPECT_EQ( 1000000, devProduct->getValue() );

		EXPECT_EQ( "Michael CSharp Senior", employees[1]->getName() );
		EXPECT_EQ( 4000, employees[1]->getSalary() );
		EXPECT_EQ( "Developer", employees[1]->getRole() );
		EXPECT_EQ( NULL, employees[1]->getLeading() );

		ASSERT_EQ( 1, employees[1]->getWorking().getSize() );
		ASSERT_NO_THROW( devProduct = co::cast<dom::IProduct>( employees[1]->getWorking()[0] ) );
		EXPECT_EQ( "Software2.0", devProduct->getName() );
		EXPECT_EQ( 1000000, devProduct->getValue() );

		EXPECT_EQ( "Richard Scrum Master", employees[2]->getName() );
		EXPECT_EQ( 10000, employees[2]->getSalary() );
		EXPECT_EQ( "Manager", employees[2]->getRole() );

		EXPECT_EQ( 0, employees[2]->getWorking().getSize() );
		ASSERT_TRUE( employees[2]->getLeading() != NULL );
		ASSERT_NO_THROW( devProduct = co::cast<dom::IProduct>( employees[2]->getLeading() ) );

		EXPECT_EQ( "Software2.0", devProduct->getName() );
		EXPECT_EQ( 1000000, devProduct->getValue() );

		EXPECT_EQ( "John Cplusplus Experienced", employees[3]->getName() );
		EXPECT_EQ( 5000, employees[3]->getSalary() );
		EXPECT_EQ( "Developer", employees[3]->getRole() );
		EXPECT_EQ( NULL, employees[3]->getLeading() );

		dom::IService* devService; 

		ASSERT_EQ( 1, employees[3]->getWorking().getSize() );
		ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[3]->getWorking()[0] ) );
		EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
		EXPECT_EQ( 50000.0, devService->getMonthlyIncome() );

		EXPECT_EQ( "Jacob Lua Junior", employees[4]->getName() );
		EXPECT_EQ( 3000, employees[4]->getSalary() );
		EXPECT_EQ( "Developer", employees[4]->getRole() );
		EXPECT_EQ( NULL, employees[4]->getLeading() );

		ASSERT_EQ( 1, employees[4]->getWorking().getSize() );
		ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[4]->getWorking()[0] ) );
		EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
		EXPECT_NEAR( 50000.0, devService->getMonthlyIncome(), 0.01 );
	}

	void applyChanges( ca::ISpace* space )
	{
		co::IObject*  root = space->getRootObject();
		dom::ICompany* company = root->getService<dom::ICompany>();

		co::TSlice<dom::IEmployee*> employees = company->getEmployees();

		//employees[0]->setName( "Joseph Java Newbie JR." ); //change without notify

		std::vector<dom::IEmployee*> changedEmployees;

		changedEmployees.push_back( employees[0] );

		changedEmployees.push_back( employees[1] );

		dom::IEmployee* promoted = employees[2];

		employees[2]->setRole("Development Manager");
		//space->addChange( employees[2] );
		//space->notifyChanges();

		changedEmployees.push_back( employees[3] );

		changedEmployees.push_back( employees[4] );

		//more than one change in a object
		employees[4]->setName( "Jacob Lua Son" );
		employees[4]->setSalary( 4000 );
		space->addChange( employees[4] );
		//space->notifyChanges();

		dom::IService* devService = co::cast<dom::IService>( employees[4]->getWorking()[0] );

		devService->setMonthlyIncome( 60000 );
		space->addChange( devService );
		//space->notifyChanges();

		company->setEmployees( changedEmployees );

		co::IObject* newCEOObj = co::newInstance( "dom.Employee" );
		dom::IEmployee* newCEO = newCEOObj->getService<dom::IEmployee>();
		newCEO->setName( "newCEO" );
		newCEO->setRole( "CEO" );
		newCEO->setSalary( 1000000 );

		root->setService( "ceo", newCEO );
		newCEO->setLeading( devService );
	
		space->addChange( company );
		space->addChange( root );

		space->notifyChanges();
	}

	void applyChangeOnNewObject( ca::ISpace* space )
	{
		co::IObject* root = space->getRootObject();
		dom::IEmployee* ceoEmployee = co::cast<dom::IEmployee>( root->getService( "ceo" ) );
		ceoEmployee->setName( "newCEO Super" );
		ceoEmployee->setSalary( 200000 );
		space->addChange( ceoEmployee );
		space->notifyChanges();
	}

	void checkSpaceAfterChange( co::IObject* objRest )
	{
		dom::ICompany* company = objRest->getService<dom::ICompany>();
		ASSERT_TRUE( company != NULL );

		co::TSlice<dom::IEmployee*> employees = company->getEmployees();
		ASSERT_EQ( 4, employees.getSize() );
	
		EXPECT_EQ( "Joseph Java Newbie", employees[0]->getName() );
		EXPECT_EQ( 1000, employees[0]->getSalary() );
		EXPECT_EQ( "Developer", employees[0]->getRole() );
		EXPECT_EQ( NULL, employees[0]->getLeading() );

		dom::IProduct* devProduct;

		ASSERT_EQ( 1, employees[0]->getWorking().getSize() );
		ASSERT_NO_THROW( devProduct = co::cast<dom::IProduct>( employees[0]->getWorking()[0] ) );
		EXPECT_EQ( "Software2.0", devProduct->getName() );
		EXPECT_EQ( 1000000, devProduct->getValue() );

		EXPECT_EQ( "Michael CSharp Senior", employees[1]->getName() );
		EXPECT_EQ( 4000, employees[1]->getSalary() );
		EXPECT_EQ( "Developer", employees[1]->getRole() );
		EXPECT_EQ( NULL, employees[1]->getLeading() );

		ASSERT_EQ( 1, employees[1]->getWorking().getSize() );
		ASSERT_NO_THROW( devProduct = co::cast<dom::IProduct>( employees[1]->getWorking()[0] ) );
		EXPECT_EQ( "Software2.0", devProduct->getName() );
		EXPECT_EQ( 1000000, devProduct->getValue() );

		EXPECT_EQ( "John Cplusplus Experienced", employees[2]->getName() );
		EXPECT_EQ( 5000, employees[2]->getSalary() );
		EXPECT_EQ( "Developer", employees[2]->getRole() );
		EXPECT_EQ( NULL, employees[2]->getLeading() );

		dom::IService* devService;

		ASSERT_EQ( 1, employees[2]->getWorking().getSize() );
		ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[2]->getWorking()[0] ) );
		EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
		EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );

		EXPECT_EQ( "Jacob Lua Son", employees[3]->getName() );
		EXPECT_EQ( 4000, employees[3]->getSalary() );
		EXPECT_EQ( "Developer", employees[3]->getRole() );
		EXPECT_EQ( NULL, employees[3]->getLeading() );

		ASSERT_EQ( 1, employees[3]->getWorking().getSize() );
		ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[3]->getWorking()[0] ) );
		EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
		EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );

		dom::IEmployee* ceoEmployee = co::cast<dom::IEmployee>( objRest->getService( "ceo" ) );

		EXPECT_EQ( "newCEO", ceoEmployee->getName() );
		EXPECT_EQ( 1000000, ceoEmployee->getSalary() );
		EXPECT_EQ( "CEO", ceoEmployee->getRole() );

		EXPECT_EQ( 0, ceoEmployee->getWorking().getSize() );
		ASSERT_TRUE( ceoEmployee->getLeading() != NULL );
		ASSERT_NO_THROW( devService = co::cast<dom::IService>( ceoEmployee->getLeading() ) );

		EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
		EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );
	}

	void removeAndReturnObject( ca::ISpace* space )
	{
		co::IObject*  root = space->getRootObject();
		dom::ICompany* company = root->getService<dom::ICompany>();

		co::TSlice<dom::IEmployee*> employees = company->getEmployees();

		std::vector<dom::IEmployee*> changedEmployess;
		dom::IEmployee* removedEmployee = employees[0];

		for( int i = 1; i < employees.getSize(); i++ )
		{
			changedEmployess.push_back( employees[i] );
		}
		company->setEmployees( changedEmployess );
		space->addChange( company );

		space->notifyChanges();

		changedEmployess.insert( changedEmployess.begin(), removedEmployee );
		company->setEmployees( changedEmployess );
		space->addChange( company );
		space->notifyChanges();
	}

	std::string getObjectName( co::IObject* obj )
	{
		co::IComponent* component = obj->getComponent();
		co::IService* service = obj->getServiceAt( component->getPorts()[0] );
		
		co::IField* field = co::cast<co::IField>( service->getInterface()->getMember( "name" ) );

		std::string name;

		co::Any value( name );
		if( field != NULL )
		{
			field->getOwner()->getReflector()->getField( service, field, value );

			return name;
		}
		return "no field name";

	}

	co::RefPtr<ca::ISpace> createLocalSpace( const std::string& modelName )
	{
		co::IObject* universeObj = co::newInstance( "ca.Universe" );
		co::IObject* modelObj = co::newInstance( "ca.Model" );
		ca::IModel* model = modelObj->getService<ca::IModel>();
		model->setName( modelName );
		universeObj->setService( "model", model );
		co::IObject* localSpaceObj = co::newInstance( "ca.Space" );
		localSpaceObj->setService( "universe", universeObj->getService<ca::IUniverse>() );
		co::RefPtr<ca::ISpace> localSpace = localSpaceObj->getService<ca::ISpace>();

		return localSpace;
	}

	// name should identify the objects in this specific case, cause our case test doesn't have name conflicts and, except for the Company, all objects has a name.

	void compareAllNames( co::Slice<co::IObject*> rangeOne, co::Slice<co::IObject*> rangeAnother )
	{
		for( int i = 0; i < rangeOne.getSize(); i++ )
		{
			EXPECT_EQ( getObjectName( rangeOne[i] ), getObjectName( rangeAnother[i] ) );
		}
	}

	void compareChanges( ca::IGraphChanges* one, ca::IGraphChanges* another  )
	{

		ASSERT_EQ( one->getAddedObjects().getSize(), another->getAddedObjects().getSize() );

		compareAllNames( one->getAddedObjects().asSlice(), another->getAddedObjects().asSlice() );

		ASSERT_EQ( one->getChangedObjects().getSize(), another->getChangedObjects().getSize() );
		
		ASSERT_EQ( one->getRemovedObjects().getSize(), another->getRemovedObjects().getSize() );

		compareAllNames( one->getAddedObjects().asSlice(), another->getAddedObjects().asSlice() );
	}

};

TEST_F( ReplicaTests, clientServerConfigErrorTests )
{
	co::RefPtr<co::IObject> spacePublisherObj = co::newInstance( "flow.SpacePublisher" );

	co::RefPtr<flow::ISpacePublisher> spacePublisher = spacePublisherObj->getService<flow::ISpacePublisher>();

	ASSERT_THROW( spacePublisher->publish(), co::IllegalStateException ); // space not set

	ASSERT_THROW( spacePublisher->setSpace( NULL ), co::IllegalArgumentException );

	spacePublisher->setSpace( _space.get() );

	ASSERT_NO_THROW( spacePublisher->publish() ); // just a warning, no subscribers

	co::RefPtr<co::IObject> spaceSubscriberObj = co::newInstance( "flow.SpaceSubscriber" );
	co::RefPtr<flow::ISpaceSubscriber> spaceSubscriber = spaceSubscriberObj->getService<flow::ISpaceSubscriber>();

	spaceSubscriber->setSpace( createLocalSpace( "otherName" ).get() );

	ASSERT_THROW( spacePublisher->addSubscriber( spaceSubscriber.get() ), co::IllegalStateException ); // incompatible model names

	spaceSubscriber->setSpace( createLocalSpace( "dom" ).get() );

	ASSERT_NO_THROW( spacePublisher->addSubscriber( spaceSubscriber.get() ) ); // ok

}

TEST_F( ReplicaTests, clientSpacePublisherTests )
{

	co::IObject* spacePublisherObj = co::newInstance( "flow.SpacePublisher" );

	flow::ISpacePublisher* spacePublisher = spacePublisherObj->getService<flow::ISpacePublisher>();
	spacePublisher->setSpace( _space.get() );
	
	LocalSpaceObserver* server = new LocalSpaceObserver();
	_space->addGraphObserver( server );

	co::IObject* spaceSubscriberObj = co::newInstance( "flow.SpaceSubscriber" );
	
	co::RefPtr<ca::ISpace> localSpace = createLocalSpace( "dom" );

	co::RefPtr<flow::ISpaceSubscriber> spaceSubscriber = spaceSubscriberObj->getService<flow::ISpaceSubscriber>();


	ASSERT_NO_THROW( spacePublisher->addSubscriber( spaceSubscriber.get() ) );
	localSpace->initialize( spaceSubscriber->getRootObject() );
	localSpace->notifyChanges();
	spaceSubscriber->setSpace( localSpace.get() );

	// initial copy test
	checkInitialSpace( spaceSubscriber->getRootObject() );

	//============================================= changes to server space
	applyChanges( _space.get() );
	
	LocalSpaceObserver* client = new LocalSpaceObserver();
	localSpace->addGraphObserver( client );

	spacePublisher->publish();

	compareChanges( server->getLastChanges(), client->getLastChanges() );

	//======================================================
	//client after change
	checkSpaceAfterChange( localSpace->getRootObject() );

	ASSERT_FALSE( client->getLastChanges() == NULL );

	//=== remove and return
	removeAndReturnObject( _space.get() );
	spacePublisher->publish();
	//it shouldn't change at all
	checkSpaceAfterChange( localSpace->getRootObject() );

	applyChangeOnNewObject( _space.get() );
	spacePublisher->publish();

	compareChanges( server->getLastChanges(), client->getLastChanges() );

	dom::IEmployee* ceo = co::cast<dom::IEmployee>( localSpace->getRootObject()->getService( "ceo" ) );
	EXPECT_EQ( 200000, ceo->getSalary() );
	EXPECT_EQ( "newCEO Super", ceo->getName() );

}


TEST_F( ReplicaTests, replicaAfterChangeTests )
{

	co::IObject* spacePublisherObj = co::newInstance( "flow.SpacePublisher" );
	flow::ISpacePublisher* spacePublisher = spacePublisherObj->getService<flow::ISpacePublisher>();
	spacePublisher->setSpace( _space.get() );
	//============================================= changes to server space
	applyChanges( _space.get() );

	ASSERT_NO_THROW( spacePublisher->publish() ); 

	//======================================================

	co::RefPtr<ca::ISpace> localSpace = createLocalSpace( "dom" );

	co::IObject* spaceSubscriberObj = co::newInstance( "flow.SpaceSubscriber" );
	
	co::RefPtr<flow::ISpaceSubscriber> spaceSubscriber = spaceSubscriberObj->getService<flow::ISpaceSubscriber>();
	spaceSubscriber->setSpace( localSpace.get() );
	
	ASSERT_NO_THROW( spacePublisher->addSubscriber( spaceSubscriber.get() ) );

	checkSpaceAfterChange( spaceSubscriberObj->getService<flow::ISpaceSubscriber>()->getRootObject() );
	
}

TEST_F( ReplicaTests, multipleClientsInitializeFromServerTests )
{

	co::IObject* spacePublisherObj = co::newInstance( "flow.SpacePublisher" );
	
	flow::ISpacePublisher* spacePublisher = spacePublisherObj->getService<flow::ISpacePublisher>();
	spacePublisher->setSpace( _space.get() );

	co::IObject* replicaObj[3];
	std::stringstream ss;

	std::vector<ca::ISpaceRef> localSpaces;

	for( int i = 0; i < 3; i++ )
	{	
		replicaObj[i] = co::newInstance( "flow.SpaceSubscriber" );
		
		ss << "client_" << i;
		std::string clientKey = ss.str();
		ss.str("");
		ss.clear();
		co::RefPtr<flow::ISpaceSubscriber> replica = replicaObj[i]->getService<flow::ISpaceSubscriber>();

		localSpaces.push_back( createLocalSpace( "dom" ) );
		flow::ISpaceSubscriber* spaceSubscriber = replicaObj[i]->getService<flow::ISpaceSubscriber>();
		spaceSubscriber->setSpace( localSpaces[i].get() );
		
		ASSERT_NO_THROW( spacePublisher->addSubscriber( spaceSubscriber )  );
	}


	// initial copy test
	for( int i = 0; i < 3; i++ )
	{
		checkInitialSpace( replicaObj[i]->getService<flow::ISpaceSubscriber>()->getRootObject() );
	}
	

	//============================================= changes to server space
	applyChanges( _space.get() );

	ASSERT_NO_THROW( spacePublisher->publish() ); 

	//======================================================
	//client after change
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( localSpaces[i]->getRootObject() );
	}

		//=== remove and return
	removeAndReturnObject( _space.get() );
	spacePublisher->publish();
	//it shouldn't change at all
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( localSpaces[i]->getRootObject()  );
	}

	applyChangeOnNewObject( _space.get() );
	spacePublisher->publish();

	for( int i = 0; i < 3; i++ )
	{
		dom::IEmployee* ceo = co::cast<dom::IEmployee>( localSpaces[i]->getRootObject()->getService( "ceo" ) );
		EXPECT_EQ( 200000, ceo->getSalary() );
		EXPECT_EQ( "newCEO Super", ceo->getName() );
	}

	
}

TEST_F( ReplicaTests, multipleClientsLateInitializeTests )
{

	co::IObject* spacePublisherObj = co::newInstance( "flow.SpacePublisher" );
	
	flow::ISpacePublisher* spacePublisher = spacePublisherObj->getService<flow::ISpacePublisher>();
	spacePublisher->setSpace( _space.get() );

	co::IObjectRef replicaObj[2];
	std::stringstream ss;

	std::vector<ca::ISpaceRef> localSpaces;

	for( int i = 0; i < 2; i++ )
	{	
		replicaObj[i] = co::newInstance( "flow.SpaceSubscriber" );
		
		ss << "client_" << i;
		std::string clientKey = ss.str();
		ss.str("");
		ss.clear();
		co::RefPtr<flow::ISpaceSubscriber> replica = replicaObj[i]->getService<flow::ISpaceSubscriber>();

		localSpaces.push_back( createLocalSpace( "dom" ) );
		flow::ISpaceSubscriber* spaceSubscriber = replicaObj[i]->getService<flow::ISpaceSubscriber>();
		spaceSubscriber->setSpace( localSpaces[i].get() );
	}

	flow::ISpaceSubscriber* subscriberInit = replicaObj[0]->getService<flow::ISpaceSubscriber>();
	flow::ISpaceSubscriber* subscriberLate = replicaObj[1]->getService<flow::ISpaceSubscriber>();


	ASSERT_NO_THROW( spacePublisher->addSubscriber( subscriberInit )  );

	// initial copy test
	for( int i = 0; i < 1; i++ )
	{
		checkInitialSpace( replicaObj[i]->getService<flow::ISpaceSubscriber>()->getRootObject() );
	}
	

	//============================================= changes to server space
	applyChanges( _space.get() );

	ASSERT_NO_THROW( spacePublisher->publish() ); 

	//======================================================
	//client after change
	for( int i = 0; i < 1; i++ )
	{
		checkSpaceAfterChange( localSpaces[i]->getRootObject() );
	}

		//=== remove and return
	removeAndReturnObject( _space.get() );
	spacePublisher->publish();

	ASSERT_NO_THROW( spacePublisher->addSubscriber( subscriberLate ) );


	//it shouldn't change at all
	for( int i = 0; i < 2; i++ )
	{
		checkSpaceAfterChange( localSpaces[i]->getRootObject()  );
	}

	applyChangeOnNewObject( _space.get() );
	spacePublisher->publish();

	for( int i = 0; i < 2; i++ )
	{
		dom::IEmployee* ceo = co::cast<dom::IEmployee>( localSpaces[i]->getRootObject()->getService( "ceo" ) );
		EXPECT_EQ( 200000, ceo->getSalary() );
		EXPECT_EQ( "newCEO Super", ceo->getName() );
	}
}