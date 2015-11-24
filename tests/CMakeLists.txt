# Locate the GTest Framework.
find_nuget ( siviep_gtest )

# We can only add the test projects if GTest is available.
if( SIVIEP_GTEST_FOUND )

	# On UNIX, GTest generally depends on pthreads, but CMake does not
	# add pthreads to SIVIEP_GTEST_LIBRARIES. So we do it...
	if( UNIX )
		LIST( APPEND SIVIEP_GTEST_LIBRARIES pthread )
	endif()

	# Check if Valgrind is available for the memory check tests
	FIND_PROGRAM( VALGRIND_COMMAND valgrind )
	if( NOT VALGRIND_COMMAND )
		MESSAGE( "WARNING: valgrind not found - all memory tests have been disabled!" )
	endif()
	
	add_subdirectory( src )

else()
	MESSAGE( "WARNING: gtest not found - all tests have been disabled!" )
endif()
