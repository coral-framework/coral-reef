#include <gtest/gtest.h>

#include <stubs/ISimpleTypes.h>
#include <stubs/IComplexTypes.h>
#include <stubs/IReferenceTypes.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>

#include <flow/IClientSpace.h>

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

namespace rpc {

TEST( SmokeTests, clientServerSpaceTests )
{
	//client setup
	co::IObject* nodeObj = co::newInstance( "rpc.Node" );
    
    rpc::INode* node = nodeObj->getService<rpc::INode>();
    
    rpc::ITransport* transport = co::newInstance( "zmq.ZMQTransport" )->getService<rpc::ITransport>();
    
    nodeObj->setService( "transport", transport );
    
    node->start( "tcp://*:4021", "tcp://localhost:4021" );

	co::IObject* obj = co::newInstance( "dom.Company" );
	co::IObject* replicaObj = co::newInstance( "flow.ClientSpace" );
	replicaObj->setService( "clientNode", node );

	co::IObject* universeObj = co::newInstance( "ca.Universe" );

	co::IObject* modelObj = co::newInstance( "ca.Model" );
	ca::IModel* model = modelObj->getService<ca::IModel>();
	model->setName( "dom" );
	
	universeObj->setService( "model", model );

	std::vector<dom::IEmployee*> changedEmployees;
	
	co::RefPtr<flow::IClientSpace> replica = replicaObj->getService<flow::IClientSpace>();

	co::RefPtr<ca::ISpace> spaceRestored;
	co::RefPtr<ca::IUniverse> universe = universeObj->getService<ca::IUniverse>();
	replicaObj->setService( "universe", universe.get() );

	ASSERT_NO_THROW( replica->initialize( "tcp://localhost:4020", "published" ) );
	
	spaceRestored = replica->getSpace();
	co::IObject* objRest = spaceRestored->getRootObject();
	
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

	co::IObject* controlObj = node->findRemoteInstance( "stubs.TestComponent", "control", "tcp://localhost:4020" );
	

	co::RefPtr<stubs::ISimpleTypes> control = controlObj->getService<stubs::ISimpleTypes>();
	control->setStoredInt( 1 );
	control->getStoredInt();
	while( control->getStoredInt() == 1 ){}
	co::RefPtr<ca::ISpace> spaceRestored2 = replica->getSpace();

	ASSERT_TRUE( spaceRestored2.isValid() );

	co::RefPtr<co::IObject> objRest2 = spaceRestored2->getRootObject();
	
	company = objRest2->getService<dom::ICompany>();
	ASSERT_TRUE( company != NULL );

	employees = company->getEmployees();
	ASSERT_EQ( 4, employees.getSize() );
	
	EXPECT_EQ( "Joseph Java Newbie", employees[0]->getName() );
	EXPECT_EQ( 1000, employees[0]->getSalary() );
	EXPECT_EQ( "Developer", employees[0]->getRole() );
	EXPECT_EQ( NULL, employees[0]->getLeading() );

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

	ASSERT_EQ( 1, employees[2]->getWorking().getSize() );
	ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[2]->getWorking()[0] ) );
	EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
	EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );

	EXPECT_EQ( "Jacob Lua Junior", employees[3]->getName() );
	EXPECT_EQ( 4000, employees[3]->getSalary() );
	EXPECT_EQ( "Developer", employees[3]->getRole() );
	EXPECT_EQ( NULL, employees[3]->getLeading() );

	ASSERT_EQ( 1, employees[3]->getWorking().getSize() );
	ASSERT_NO_THROW( devService = co::cast<dom::IService>( employees[3]->getWorking()[0] ) );
	EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
	EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );

	dom::IEmployee* ceoEmployee = co::cast<dom::IEmployee>( objRest2->getService( "ceo" ) );

	EXPECT_EQ( "newCEO", ceoEmployee->getName() );
	EXPECT_EQ( 1000000, ceoEmployee->getSalary() );
	EXPECT_EQ( "CEO", ceoEmployee->getRole() );

	EXPECT_EQ( 0, ceoEmployee->getWorking().getSize() );
	ASSERT_TRUE( ceoEmployee->getLeading() != NULL );
	ASSERT_NO_THROW( devService = co::cast<dom::IService>( ceoEmployee->getLeading() ) );

	EXPECT_EQ( "Software1.0 Maintenance", devService->getName() );
	EXPECT_EQ( 60000.0, devService->getMonthlyIncome() );

	node->stop();

}

TEST( SmokeTests, simpleTypesTest )
{
    // Creates the node instance
    co::IObject* nodeObj = co::newInstance( "rpc.Node" );
    
    // Gets the INode interface, which is the interface with remote hosts
    rpc::INode* node = nodeObj->getService<rpc::INode>();
    
    // Creates the instance responsible for the transport layer
    rpc::ITransport* transport = co::newInstance( "zmq.ZMQTransport" )->getService<rpc::ITransport>();
    
    // The node instance needs the transport layer to communicate
    nodeObj->setService( "transport", transport );
    
    /* Even though this node wont be acting as a server, it is necessary to be bound to a public
     address. This happens because whenever a local service is passed as a reference argument, 
     the receiver will need to send messages to this node. */
    node->start( "tcp://*:4021", "tcp://localhost:4021" );
    
    // instantiates a stubs.TestComponent in host bound to the given address
    co::RefPtr<co::IObject> remoteInstance = node->newRemoteInstance( "stubs.TestComponent",
                                                                       "tcp://localhost:4020" );
    
    // Gets the stubs.ISimpleTypes interface from the remote instance
    stubs::ISimpleTypes* simple = remoteInstance->getService<stubs::ISimpleTypes>();
    
    // simple get and set test
    simple->setDouble( 0.1 );
    EXPECT_EQ( simple->getStoredDouble(), 0.1 );

	node->stop();
}

} // namespace rpc
