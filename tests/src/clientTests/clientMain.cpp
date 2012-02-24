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

		co::IObject* obj = co::newInstance( "reef.ClientNode" );
		reef::IClientNode* client = obj->getService<reef::IClientNode>();
		co::IObject* toto = client->newRemoteInstance( "toto.Toto", "tcp://localhost:4020" );
		toto::IToto* totoService = toto->getService<toto::IToto>();
		/*totoService->printInt( 10267399 );
		totoService->printString( "The the quick brown fox fox jumped over the lazy dog dog" );
		totoService->printDouble( 0.000123321 );
		totoService->printDouble( 12345678.987654 );*/
    
        totoService->setNumber( 19 );
        totoService->printMethod3();
        co::int32 number = totoService->numberGet();
        std::cout << number << " is the number" << std::endl;
        totoService->setNumber( 33 );
        totoService->printMethod3();
        number = totoService->getNumber();
        std::cout << number << " is the number" << std::endl;
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
