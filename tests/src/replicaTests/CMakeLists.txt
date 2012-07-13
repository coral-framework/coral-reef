
project( replica )

CORAL_GENERATE_MAPPINGS( _GENERATED_MAPPINGS "rpcTests.ITestSetup" )

# set the tests 'module' dir as the CORAL_PATH along with the CORAL_ROOT
set( CORAL_PATH ${CORAL_PATH} 
	"${CMAKE_SOURCE_DIR}/tests/modules" 
	"${CMAKE_SOURCE_DIR}/modules" 
	)

	
	
# Pass the CORAL_PATH as a precompiler definition to all subprojects
CORAL_GET_PATH_STRING( CORAL_PATH_STR )
set_property( DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS "CORAL_PATH=\"${CORAL_PATH_STR}\"" )

################################################################################
# Build the test executable
################################################################################

include_directories(
	${GTEST_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/reef/rpc/generated
	${CMAKE_SOURCE_DIR}/src/reef/rpc
	${CMAKE_BINARY_DIR}/src/dso/generated
	${CMAKE_SOURCE_DIR}/src/dso
	${rpcTests_BINARY_DIR}/generated
	${dom_BINARY_DIR}/generated
)

# Gather test source files in the current directory
file( GLOB _TESTS *Tests.cpp )

file( GLOB _HEADERS CompanySpace.h )
file( GLOB _SOURCES CompanySpace.cpp Main.cpp )

# Create test executable
add_executable( replica EXCLUDE_FROM_ALL ${_TESTS} ${_HEADERS} ${_SOURCES} ${_GENERATED_MAPPINGS} )
add_dependencies( replica dom rpcTests rpc dso)

CORAL_TARGET( replica )

set_target_properties( replica PROPERTIES
	PROJECT_LABEL "replica"
)

target_link_libraries( replica ${GTEST_LIBRARIES} ${CORAL_LIBRARIES} ${CALCIUM_LIBRARIES} ${REEF_LIBRARIES} )

source_group( "@Generated" FILES ${_GENERATED_FILES} )

################################################################################
# Register the test
################################################################################

add_test(
		NAME replica
		COMMAND $<TARGET_FILE:replica> --gtest_output=xml:../output/replica_$<CONFIGURATION>.xml
	)
	CORAL_TEST_ENVIRONMENT( replica )
