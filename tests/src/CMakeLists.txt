################################################################################
# Common settings for all test subprojects
################################################################################

# set the tests 'module' dir as the CORAL_PATH along with the CORAL_ROOT
set( CORAL_PATH ${CORAL_PATH} "${CMAKE_SOURCE_DIR}/tests/modules" )

# Pass the CORAL_PATH as a precompiler definition to all subprojects
CORAL_GET_PATH_STRING( CORAL_PATH_STR )
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "CORAL_PATH=\"${CORAL_PATH_STR}\"" )

add_subdirectory( rpcTests )

set( RPC_ONLY "OFF" CACHE BOOL "No flow module, rpc only" )
if( RPC_ONLY STREQUAL "OFF" )
	add_subdirectory( flowTests )
endif()

set( BUILD_MANUAL_TESTS "OFF" CACHE BOOL "Manual client server testing" )
if( BUILD_MANUAL_TESTS STREQUAL "ON" )
	message( "Building manual tests" )
	add_subdirectory( manualTests )
endif()

add_custom_target( tests DEPENDS rpcTests flowTests )