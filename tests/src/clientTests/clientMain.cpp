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
		
		std::vector<co::int32> numberList;
		numberList.push_back( 0 );
		numberList.push_back( 1 );
		numberList.push_back( 2 );
		numberList.push_back( 4 );
		numberList.push_back( 8 );
		numberList.push_back( 100 );
		numberList.push_back( 101 );
    
		std::vector<std::string> stringList;
		stringList.push_back( "The" );
		stringList.push_back( "the" );
		stringList.push_back( "quick" );
		stringList.push_back( "brown" );
		stringList.push_back( "fox" );
		stringList.push_back( "fox" );
		stringList.push_back( "jumped" );
		stringList.push_back( "over" );
		stringList.push_back( "the" );
		stringList.push_back( "lazy" );
		stringList.push_back( "dog" );
		stringList.push_back( "dog" );
    
		//totoService->printNumberList( numberList );
		//totoService->printStringList( stringList );
		totoService->printHybridList( numberList, stringList );
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
