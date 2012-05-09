/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
#include <gtest/gtest.h>

#include "Message.pb.h"
#include "ModuleInstaller.h"

int main( int argc, char** argv )
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
	testing::InitGoogleTest( &argc, argv );

	// skip dlclose() so we get proper valgrind reports
	co::LibraryManager::setNoDlClose();

	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();
    reef::rpc::ModuleInstaller& moduleInstaller = reef::rpc::ModuleInstaller::instance();
    moduleInstaller.install();
    
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

    try
    {
        moduleInstaller.uninstall();
    }
    catch( std::exception& e )
    {
    }
    
    google::protobuf::ShutdownProtobufLibrary();
	co::shutdown();

	return res;	
}
