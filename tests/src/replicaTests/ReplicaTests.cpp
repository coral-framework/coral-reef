/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include <gtest/gtest.h>

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/RefPtr.h>
#include <co/Log.h>
#include <co/IllegalStateException.h>
#include <co/IllegalArgumentException.h>

#include "CompanySpace.h"

#include <dom/ICompany.h>
#include <dom/IEmployee.h>
#include <dom/IProduct.h>
#include <dom/IService.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <ca/ISpace.h>
#include <ca/IModel.h>
#include <ca/IUniverse.h>
#include <ca/IOException.h>

#include <dso/IClientSpace.h>
#include <dso/IServerSpace.h>
#include <dso/IRemoteSpaceObserver.h>

#include <rpcTests/ITestSetup.h>

class ReplicaTests : public CompanySpace
{
public:

	void SetUp()
	{
		CompanySpace::SetUp();

		co::RefPtr<co::IObject> setupObj = co::newInstance( "rpcTests.TestSetup" );
		_setup = setupObj->getService<rpcTests::ITestSetup>();
		_setup->initTest( 2 );

		_server = _setup->getNode( 1 );
		_client = _setup->getNode( 2 );

	}

	void TearDown()
	{
		CompanySpace::TearDown();
		_setup->tearDown();

	}

	void checkInitialSpace( ca::ISpace* space )
	{
		co::IObject* objRest = space->getRootObject();
	
		dom::ICompany* company = objRest->getService<dom::ICompany>();
		ASSERT_TRUE( company != NULL );

		co::Range<dom::IEmployee* const> employees = company->getEmployees();
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

		co::Range<dom::IEmployee* const> employees = company->getEmployees();

		//employees[0]->setName( "Joseph Java Newbie JR." ); //change without notify

		std::vector<dom::IEmployee*> changedEmployees;

		changedEmployees.push_back( employees[0] );

		changedEmployees.push_back( employees[1] );

		dom::IEmployee* promoted = employees[2];

		employees[2]->setRole("Development Manager");
		space->addChange( employees[2] );
		space->notifyChanges();

		changedEmployees.push_back( employees[3] );

		changedEmployees.push_back( employees[4] );

		//more than one change in a object
		employees[4]->setName( "Jacob Lua Son" );
		employees[4]->setSalary( 4000 );
		space->addChange( employees[4] );
		space->notifyChanges();

		dom::IService* devService = co::cast<dom::IService>( employees[4]->getWorking()[0] );

		devService->setMonthlyIncome( 60000 );
		space->addChange( devService );
		space->notifyChanges();

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

	void checkSpaceAfterChange( ca::ISpace* space )
	{
		co::RefPtr<co::IObject> objRest = space->getRootObject();
	
		dom::ICompany* company = objRest->getService<dom::ICompany>();
		ASSERT_TRUE( company != NULL );

		co::Range<dom::IEmployee* const> employees = company->getEmployees();
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

		co::Range<dom::IEmployee* const> employees = company->getEmployees();

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


	co::RefPtr<rpcTests::ITestSetup> _setup;

	co::RefPtr<reef::rpc::INode> _server;
	co::RefPtr<reef::rpc::INode> _client;

};

TEST_F( ReplicaTests, clientServerConfigErrorTests )
{
	co::RefPtr<co::IObject> serverSpaceObj = co::newInstance( "dso.ServerSpace" );

	co::RefPtr<dso::IServerSpace> serverSpace = serverSpaceObj->getService<dso::IServerSpace>();

	ASSERT_THROW( serverSpace->publishSpace( _space.get(), "anyName" ), co::IllegalStateException ); // server node not set

	serverSpaceObj->setService( "serverNode", _server.get() );

	ASSERT_THROW( serverSpace->publishSpace( NULL, "anyName" ), co::IllegalArgumentException ); // NULL space

	ASSERT_THROW( serverSpace->publishSpace( _space.get(), "" ), co::IllegalArgumentException ); // empty name

	ASSERT_THROW( serverSpace->getPublishedSpaceData(), co::IllegalStateException ); // no space has been published yet

	ASSERT_NO_THROW( serverSpace->publishSpace( _space.get(), "publishedOk" ) ); // publishing ok

	co::RefPtr<co::IObject> clientSpaceObj = co::newInstance( "dso.ClientSpace" );
	co::RefPtr<dso::IClientSpace> clientSpace = clientSpaceObj->getService<dso::IClientSpace>();

	co::IObject* universeObj = co::newInstance( "ca.Universe" );

	co::RefPtr<co::IObject> modelObj = co::newInstance( "ca.Model" );
	ca::IModel* model = modelObj->getService<ca::IModel>();
	model->setName( "dom" );
	
	universeObj->setService( "model", model );
	co::RefPtr<ca::IUniverse> universe = universeObj->getService<ca::IUniverse>();

	ASSERT_THROW( clientSpace->initialize( "address1", "publishedOk" ), co::IllegalStateException ); // client node not set
	
	clientSpaceObj->setService( "clientNode", _client.get() );

	ASSERT_THROW( clientSpace->initialize( "address1", "publishedOk" ), co::IllegalStateException ); // universe not set

	clientSpaceObj->setService( "universe", universeObj->getService<ca::IUniverse>() );
	
	ASSERT_THROW( clientSpace->initialize( "", "" ), co::IllegalArgumentException ); // address and key not set

	ASSERT_THROW( clientSpace->initialize( "address1", "" ), co::IllegalArgumentException ); // valid address, but key not set

	ASSERT_THROW( clientSpace->initialize( "", "publishedOk" ), co::IllegalArgumentException ); // valid key, but address not set

	ASSERT_TRUE( clientSpace->getSpace() == NULL );

	ASSERT_NO_THROW( clientSpace->initialize( "address1", "publishedOk" ) ); // everything ok

	ASSERT_TRUE( clientSpace->getSpace() != NULL );


}

TEST_F( ReplicaTests, clientServerSpaceTests )
{

	co::IObject* serverSpaceObj = co::newInstance( "dso.ServerSpace" );
	serverSpaceObj->setService( "serverNode", _server.get() );

	dso::IServerSpace* serverSpace = serverSpaceObj->getService<dso::IServerSpace>();
	serverSpace->publishSpace( _space.get(), "publishedSpace" );

	co::IObject* replicaObj = co::newInstance( "dso.ClientSpace" );
	replicaObj->setService( "clientNode", _client.get() );

	co::IObject* universeObj = co::newInstance( "ca.Universe" );

	co::IObject* modelObj = co::newInstance( "ca.Model" );
	ca::IModel* model = modelObj->getService<ca::IModel>();
	model->setName( "dom" );
	
	universeObj->setService( "model", model );

	co::RefPtr<dso::IClientSpace> replica = replicaObj->getService<dso::IClientSpace>();

	co::RefPtr<ca::ISpace> spaceRestored;
	replicaObj->setService( "universe", universeObj->getService<ca::IUniverse>() );
	ASSERT_NO_THROW( replica->initialize( "address1", "publishedSpace" ) );
	ASSERT_TRUE( replica->getSpace() != NULL );

	// initial copy test

	checkInitialSpace( replica->getSpace() );

	//============================================= changes to server space
	applyChanges( serverSpace->getSpace() );

	serverSpace->notifyRemoteChanges();

	//======================================================
	//client after change
	checkSpaceAfterChange( replica->getSpace() );

	//=== remove and return
	removeAndReturnObject( serverSpace->getSpace() );
	serverSpace->notifyRemoteChanges();
	//it shouldn't change at all
	checkSpaceAfterChange( replica->getSpace() );

}

TEST_F( ReplicaTests, multipleClientsInitializeFromServerTests )
{

	co::IObject* serverSpaceObj = co::newInstance( "dso.ServerSpace" );
	serverSpaceObj->setService( "serverNode", _server.get() );

	dso::IServerSpace* serverSpace = serverSpaceObj->getService<dso::IServerSpace>();
	serverSpace->publishSpace( _space.get(), "publishedSpace" );

	co::IObject* replicaObj[3];
	std::stringstream ss;
	for( int i = 0; i < 3; i++ )
	{	
		replicaObj[i] = co::newInstance( "dso.ClientSpace" );
		replicaObj[i]->setService( "clientNode", _client.get() );

		ss << "client_" << i;
		std::string clientKey = ss.str();
		_client->publishInstance( replicaObj[i], clientKey );
		ss.str("");
		ss.clear();
		co::RefPtr<dso::IClientSpace> replica = replicaObj[i]->getService<dso::IClientSpace>();
		
		co::IObject* universeObj = co::newInstance( "ca.Universe" );
		co::IObject* modelObj = co::newInstance( "ca.Model" );
		ca::IModel* model = modelObj->getService<ca::IModel>();
		model->setName( "dom" );
		universeObj->setService( "model", model );
		replicaObj[i]->setService( "universe", universeObj->getService<ca::IUniverse>() );
		
		ASSERT_NO_THROW( serverSpace->initializeClient( _client->getPublicAddress(), clientKey )  );
	}


	// initial copy test
	for( int i = 0; i < 3; i++ )
	{
		checkInitialSpace( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}
	

	//============================================= changes to server space
	applyChanges( serverSpace->getSpace() );

	ASSERT_NO_THROW( serverSpace->notifyRemoteChanges() ); 

	//======================================================
	//client after change
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}

		//=== remove and return
	removeAndReturnObject( serverSpace->getSpace() );
	serverSpace->notifyRemoteChanges();
	//it shouldn't change at all
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}

}

TEST_F( ReplicaTests, multipleClientsServerSpaceTests  )
{

	co::IObject* serverSpaceObj = co::newInstance( "dso.ServerSpace" );
	serverSpaceObj->setService( "serverNode", _server.get() );

	dso::IServerSpace* serverSpace = serverSpaceObj->getService<dso::IServerSpace>();
	serverSpace->publishSpace( _space.get(), "publishedSpace" );

	co::IObject* replicaObj[3];
	for( int i = 0; i < 3; i++ )
	{	
		replicaObj[i] = co::newInstance( "dso.ClientSpace" );
		replicaObj[i]->setService( "clientNode", _client.get() );
		co::RefPtr<dso::IClientSpace> replica = replicaObj[i]->getService<dso::IClientSpace>();
		
		co::IObject* universeObj = co::newInstance( "ca.Universe" );
		co::IObject* modelObj = co::newInstance( "ca.Model" );
		ca::IModel* model = modelObj->getService<ca::IModel>();
		model->setName( "dom" );
		universeObj->setService( "model", model );
		replicaObj[i]->setService( "universe", universeObj->getService<ca::IUniverse>() );
		ASSERT_NO_THROW( replica->initialize( "address1", "publishedSpace" )  );
	}


	// initial copy test
	for( int i = 0; i < 3; i++ )
	{
		checkInitialSpace( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}
	

	//============================================= changes to server space
	applyChanges( serverSpace->getSpace() );

	ASSERT_NO_THROW( serverSpace->notifyRemoteChanges() ); 

	//======================================================
	//client after change
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}

		//=== remove and return
	removeAndReturnObject( serverSpace->getSpace() );
	serverSpace->notifyRemoteChanges();
	//it shouldn't change at all
	for( int i = 0; i < 3; i++ )
	{
		checkSpaceAfterChange( replicaObj[i]->getService<dso::IClientSpace>()->getSpace() );
	}

}

TEST_F( ReplicaTests, replicaAfterChangeTests )
{

	co::IObject* serverSpaceObj = co::newInstance( "dso.ServerSpace" );
	serverSpaceObj->setService( "serverNode", _server.get() );

	dso::IServerSpace* serverSpace = serverSpaceObj->getService<dso::IServerSpace>();
	serverSpace->publishSpace( _space.get(), "publishedSpace" );

	co::IObject* replicaBeforeObj = co::newInstance( "dso.ClientSpace" );
	
	replicaBeforeObj->setService( "clientNode", _client.get() );
	
	co::RefPtr<dso::IClientSpace> replica = replicaBeforeObj->getService<dso::IClientSpace>();

	co::IObject* universeObj = co::newInstance( "ca.Universe" );
	co::IObject* modelObj = co::newInstance( "ca.Model" );
	ca::IModel* model = modelObj->getService<ca::IModel>();
	model->setName( "dom" );
	universeObj->setService( "model", model );

	replicaBeforeObj->setService( "universe", universeObj->getService<ca::IUniverse>() );

	ASSERT_NO_THROW( replica->initialize( "address1", "publishedSpace" ) );


	// initial copy test
	checkInitialSpace( replicaBeforeObj->getService<dso::IClientSpace>()->getSpace() );
		

	//============================================= changes to server spaec
	applyChanges( serverSpace->getSpace() );

	ASSERT_NO_THROW( serverSpace->notifyRemoteChanges() ); 

	//======================================================
	//client after change
	checkSpaceAfterChange( replicaBeforeObj->getService<dso::IClientSpace>()->getSpace() );

	co::IObject* replicaAfterObj = co::newInstance( "dso.ClientSpace" );
	replicaAfterObj->setService( "clientNode", _client.get() );
	
	replica = replicaAfterObj->getService<dso::IClientSpace>();
	replicaAfterObj->setService( "universe", universeObj->getService<ca::IUniverse>() );
	ASSERT_NO_THROW( replica->initialize( "address1", "publishedSpace" ) );

	checkSpaceAfterChange( replica->getSpace() );

}