/*
 * Reef
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>

#include <reef/INode.h>

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/Exception.h>

#include <iostream>

int main( int argc, char** argv )
{
	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

	try
	{
		co::IObject* obj = co::newInstance( "reef.Node" );
		reef::INode* server = obj->getService<reef::INode>();
    
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
