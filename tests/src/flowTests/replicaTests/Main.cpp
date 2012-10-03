/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/reserved/LibraryManager.h>
#include <gtest/gtest.h>

#include <co/Log.h>

int main( int argc, char** argv )
{
	testing::InitGoogleTest( &argc, argv );

	// skip dlclose() so we get proper valgrind reports
	co::LibraryManager::setNoDlClose();

	co::Logger::setMinSeverity( co::LOG_WARNING );

	// set up the system
	co::addPath( CORAL_PATH );
	co::getSystem()->setup();

	int res = RUN_ALL_TESTS();
	co::shutdown();

	return res;
}
