/*
 * Reef
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>

#include <reef/INode.h>
#include <reef/ITransport.h>

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
		co::IObject* node = co::newInstance( "reef.Node" );
		reef::INode* server = node->getService<reef::INode>();
        
        reef::ITransport* transport = co::newInstance( "reef.ZMQTransport" )->getService<reef::ITransport>();
        node->setService( "transport", transport );
        
		server->start( "tcp://*:4020", "tcp://localhost:4020" );

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
