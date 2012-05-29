/*
 * Reef
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>

#include <reef/rpc/INode.h>
#include <reef/rpc/ITransport.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IObject.h>
#include <co/Exception.h>
#include <co/IComponent.h>

#include <iostream>

int main( int argc, char** argv )
{
	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

	try
	{
        // Creates the node instance
		co::IObject* node = co::newInstance( "reef.rpc.Node" );
        
        // Gets the INode interface, which is the interface with remote hosts
        reef::rpc::INode* server = node->getService<reef::rpc::INode>();
        
        // Creates the instance responsible for the transport layer
        reef::rpc::ITransport* transport = co::newInstance( "zmq.ZMQTransport" )->getService<reef::rpc::ITransport>();
        
        // The node instance needs the transport layer to communicate
        node->setService( "transport", transport );
        
        // Start the node server at given port
		server->start( "tcp://*:4020", "tcp://localhost:4020" );

        // Just update the node forever
		while( true )
			server->update();
        
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
