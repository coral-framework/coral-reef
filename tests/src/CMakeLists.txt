################################################################################
# Common settings for all test subprojects
################################################################################

# Add the tests 'module' dir to the CORAL_PATH
set( CORAL_PATH ${CORAL_PATH} "${CMAKE_SOURCE_DIR}/tests/modules" )

# Pass the CORAL_PATH as a precompiler definition to all subprojects
CORAL_GET_PATH_STRING( CORAL_PATH_STR )
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "CORAL_PATH=\"${CORAL_PATH_STR}\"" )

add_subdirectory( testModule )
add_subdirectory( tests )

set( MANUAL_TESTING ON CACHE BOOL "Manual client server testing" )
if( MANUAL_TESTING )
	add_subdirectory( clientTests )
	add_subdirectory( serverTests )
endif()