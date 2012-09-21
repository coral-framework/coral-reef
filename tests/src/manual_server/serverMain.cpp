/*
 * 
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
#include <co/reserved/OS.h>
#include <co/Log.h>

#include <rpc/INode.h>
#include <rpc/ITransport.h>

#include <flow/IServerSpace.h>

#include <dom/ICompany.h>
#include <dom/IEmployee.h>
#include <dom/IProduct.h>
#include <dom/IService.h>

#include <rpcTests/ISimpleTypes.h>

#include <ca/ISpace.h>
#include <ca/IArchive.h>
#include <ca/IUniverse.h>
#include <ca/INamed.h>
#include <ca/IModel.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IObject.h>
#include <co/Exception.h>
#include <co/IComponent.h>

#include <iostream>
#include <gtest/gtest.h>


/*void publishSpace( co::RefPtr<co::IObject>& serverSpaceObj, co::RefPtr<ca::ISpace>& space, rpc::INode* server )
{
	co::IObject* modelObj = co::newInstance( "ca.Model" );
	modelObj->getComponent();
	ca::IModel* model = modelObj->getService<ca::IModel>();
	model->setName( "dom" );

	co::IObject* archiveObj = co::newInstance( "ca.LuaArchive" );
	ca::IArchive* archive = archiveObj->getService<ca::IArchive>();
	archiveObj->getService<ca::INamed>()->setName( "initial.lua" );
	archiveObj->setService( "model", model );

	co::RefPtr<co::IObject> root = archive->restore();
	dom::ICompany* comp = root->getService<dom::ICompany>();
	co::Range<dom::IEmployee* const> employees = comp->getEmployees();

	model->loadDefinitionsFor( "dom" );
	// create an object universe and bind the model
	co::IObject* universeObj = co::newInstance( "ca.Universe" );
	ca::IUniverse* universe = universeObj->getService<ca::IUniverse>();

	universeObj->setService( "model", model );

	// create an object space and bind it to the universe
	co::RefPtr<co::IObject> spaceObj = co::newInstance( "ca.Space" );
	space = spaceObj->getService<ca::ISpace>();

	spaceObj->setService( "universe", universe );

	space->initialize( root.get() );
	space->notifyChanges();

	assert( space->getUniverse() );

	serverSpaceObj = co::newInstance( "flow.ServerSpace" );
	flow::IServerSpace* serverSpace = serverSpaceObj->getService<flow::IServerSpace>();
	serverSpaceObj->setService( "serverNode", server );
	serverSpace->publishSpace( space.get(), "published" );
}

void applyChanges( co::RefPtr<co::IObject>& serverSpaceObj, const co::RefPtr<ca::ISpace>& space )
{
	ca::ISpace* spaceOnServer = space.get();
	co::IObject*  serverRoot = spaceOnServer->getRootObject();
	dom::ICompany* company = serverRoot->getService<dom::ICompany>();

	co::Range<dom::IEmployee* const> employees = company->getEmployees();

	employees[0]->setName( "Joseph Java Newbie JR." ); //change without notify

	std::vector<dom::IEmployee*> changedEmployees;

	changedEmployees.push_back( employees[0] );

	changedEmployees.push_back( employees[1] );

	dom::IEmployee* promoted = employees[2];

	employees[2]->setRole("Development Manager");
	spaceOnServer->addChange( employees[2] );
	spaceOnServer->notifyChanges();

	changedEmployees.push_back( employees[3] );

	changedEmployees.push_back( employees[4] );

	employees[4]->setSalary( 4000 );
	spaceOnServer->addChange( employees[4] );
	spaceOnServer->notifyChanges();

	dom::IService* devService = co::cast<dom::IService>( employees[4]->getWorking()[0] );

	devService->setMonthlyIncome( 60000 );
	spaceOnServer->addChange( devService );
	spaceOnServer->notifyChanges();

	company->setEmployees( changedEmployees );

	co::IObject* newCEOObj = co::newInstance( "dom.Employee" );
	dom::IEmployee* newCEO = newCEOObj->getService<dom::IEmployee>();
	newCEO->setName( "newCEO" );
	newCEO->setRole( "CEO" );
	newCEO->setSalary( 1000000 );

	serverRoot->setService( "ceo", newCEO );
	newCEO->setLeading( devService );
	
	spaceOnServer->addChange( company );
	spaceOnServer->addChange( serverRoot );

	spaceOnServer->notifyChanges();

	serverSpaceObj->getService<flow::IServerSpace>()->notifyRemoteChanges();

	CORAL_LOG(INFO) << "Sync successfull";
}

int main( int argc, char** argv )
{
	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

	try
	{
        // Creates the node instance
		co::IObject* node = co::newInstance( "rpc.Node" );
		
		// Gets the INode interface, which is the interface with remote hosts
        rpc::INode* server = node->getService<rpc::INode>();

		// Creates the instance responsible for the transport layer
        rpc::ITransport* transport = co::newInstance( "zmq.ZMQTransport" )->getService<rpc::ITransport>();
        
        // The node instance needs the transport layer to communicate
        node->setService( "transport", transport );

		server->start( "tcp://*:4020", "tcp://localhost:4020" );

		co::RefPtr<ca::ISpace> space;
		co::RefPtr<co::IObject> serverSpaceObj;
		
		publishSpace( serverSpaceObj, space, server );

		CORAL_LOG(INFO) << "Space published";
        
        
	    // Start the node server at given port
		co::IObject* simpleTypesObj = co::newInstance( "moduleA.TestComponent" );
		rpcTests::ISimpleTypes* st = simpleTypesObj->getService<rpcTests::ISimpleTypes>();
		st->setStoredInt( 0 );

		server->publishInstance( simpleTypesObj, "control" );

        // Just update the node forever
		while( true )
		{
			server->update();
			if( st->getStoredInt() )
			{
				applyChanges( serverSpaceObj, space );
				st->setStoredInt( 0 );
			}
		}
        
	}
	catch( std::exception& e ) 
	{ 
		std::cerr << "Exception: " << e.what() << std::endl; 
	}
	catch(...)
	{
		std::cerr << "Unknown Exception" << std::endl;
	}
	co::shutdown();

	return 0;
}*/
