/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
#include <gtest/gtest.h>

int main( int argc, char** argv )
{
	testing::InitGoogleTest( &argc, argv );
    
	// skip dlclose() so we get proper valgrind reports
	co::LibraryManager::setNoDlClose();
    
	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();
    
	int res;
	try
	{
		res = RUN_ALL_TESTS();
	}
	catch( std::exception& e ) 
	{ 
		std::cerr << "Exception: " << e.what() << std::endl;
		res = 42;
	}
    
	co::shutdown();
    
	return res;	
}
