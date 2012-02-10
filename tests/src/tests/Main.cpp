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

int main( int argc, char** argv )
{
//	testing::InitGoogleTest( &argc, argv );

	// skip dlclose() so we get proper valgrind reports
//	co::LibraryManager::setNoDlClose();

	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

//	int res = RUN_ALL_TESTS();
    
    co::IObject* obj = co::newInstance( "reef.ServerNode" );
    reef::IServerNode* server = obj->getService<reef::IServerNode>();
    
    server->start( "tcp://*:4020" );

	while( true )
		server->update();
/*
    co::IObject* obj = co::newInstance( "reef.ClientNode" );
    reef::IClientNode* client = obj->getService<reef::IClientNode>();
    co::IObject* toto = client->newRemoteInstance( "toto.Toto", "tcp://10.0.24.151:4020" );
    toto::IToto* totoService = toto->getService<toto::IToto>();
    totoService->printHello();
    totoService->printWelcome();
    totoService->printMethod3();
    totoService->printMethod3();
	*/
	co::shutdown();

	return 0;
}
