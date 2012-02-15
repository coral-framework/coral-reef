/*
 * Reef
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
//#include <gtest/gtest.h>

#include <reef/IServerNode.h>
#include <reef/IClientNode.h>

#include <toto/IToto.h>

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/Exception.h>

#include <iostream>

int main( int argc, char** argv )
{
//	testing::InitGoogleTest( &argc, argv );

	// skip dlclose() so we get proper valgrind reports
//	co::LibraryManager::setNoDlClose();

	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

	try
	{
	//	int res = RUN_ALL_TESTS();
    
		co::IObject* obj = co::newInstance( "reef.ServerNode" );
		reef::IServerNode* server = obj->getService<reef::IServerNode>();
    
		server->start( "tcp://*:4020" );

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
