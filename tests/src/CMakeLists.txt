################################################################################
# Common settings for all test subprojects
################################################################################

# set the tests 'module' dir as the CORAL_PATH along with the CORAL_ROOT
set( CORAL_PATH ${CORAL_PATH} "${CMAKE_SOURCE_DIR}/tests/modules" )

# Pass the CORAL_PATH as a precompiler definition to all subprojects
CORAL_GET_PATH_STRING( CORAL_PATH_STR )
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "CORAL_PATH=\"${CORAL_PATH_STR}\"" )

add_subdirectory( moduleA )
add_subdirectory( mockReef )
add_subdirectory( unitTests )
add_subdirectory( integrationTests )
add_subdirectory( luaTests )

set( MANUAL_TESTING_DISABLED ON CACHE BOOL "Manual client server testing" )
if( NOT MANUAL_TESTING_DISABLED )
	add_subdirectory( clientTests )
	add_subdirectory( serverTests )
	# Add a custom target to build all test artifacts
	ADD_CUSTOM_TARGET( tests DEPENDS reef mockReef moduleA unitTests integrationTests
		clientTests serverTests
	)
else()
	ADD_CUSTOM_TARGET( tests DEPENDS reef mockReef moduleA unitTests integrationTests )
endif()