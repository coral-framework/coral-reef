/*
 * Reef
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
#include <co/reserved/OS.h>
#include <co/Log.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <dso/IServerSpace.h>

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


int main( int argc, char** argv )
{
	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();
    
	try
	{
        if( argc != 2 )
            throw co::Exception( "No server number provided" );
        
        int serverNumber = atoi( argv[1] );
        
        // Creates the node instance
		co::IObject* node = co::newInstance( "reef.rpc.Node" );
		
		// Gets the INode interface, which is the interface with remote hosts
        co::RefPtr<reef::rpc::INode> server = node->getService<reef::rpc::INode>();
        
		// Creates the instance responsible for the transport layer
        co::RefPtr<reef::rpc::ITransport> transport = co::newInstance( "zmq.ZMQTransport" )->getService<reef::rpc::ITransport>();
        
        // The node instance needs the transport layer to communicate
        node->setService( "transport", transport.get() );
        
        // Configuring the appropriate endpoint based on the serverNumber
        std::stringstream endpoint( std::stringstream::in | std::stringstream::out );
        endpoint << "ipc:///tmp/server" << serverNumber << ".pipe";
		server->start( endpoint.str(), endpoint.str() );
        
		CORAL_LOG(INFO) << "Server " << endpoint.str() << " Created";
        
        
	    // Start the node server at given port
		co::RefPtr<co::IObject> simpleTypesObj = co::newInstance( "rpcTests.TestComponent" );
		rpcTests::ISimpleTypes* st = simpleTypesObj->getService<rpcTests::ISimpleTypes>();
		st->setStoredInt( 0 );
        
		server->publishInstance( simpleTypesObj.get(), "control" );
        
        co::RefPtr<co::IObject> incrementer = co::newInstance( "rpcTests.Incrementer" );;
        incrementer->setService( "node", server.get() );
        
        server->publishInstance( incrementer.get(), "incrementer" );
        
        // Just update the node forever
		while( !st->getStoredInt() )
		{
			server->update();
		}
        
        server->unpublishInstance( "control" );
        server->stop();
        
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
}
