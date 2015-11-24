
project( replicaTests )

CORAL_GENERATE_MAPPINGS( _GENERATED_MAPPINGS "dom.IEmployee" "dom.IProduct" 
"dom.IService" "dom.ICompany")

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
	${SIVIEP_GTEST_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/rpc/generated
	${CMAKE_SOURCE_DIR}/src/rpc
	${CMAKE_BINARY_DIR}/src/flow/generated
	${CMAKE_SOURCE_DIR}/src/flow
)

# Gather test source files in the current directory
file( GLOB _TESTS *Tests.cpp )

file( GLOB _HEADERS CompanySpace.h LocalSpaceObserver.h )
file( GLOB _SOURCES CompanySpace.cpp LocalSpaceObserver.cpp Main.cpp )

# Create test executable
add_executable( replicaTests EXCLUDE_FROM_ALL ${_TESTS} ${_HEADERS} ${_SOURCES} ${_GENERATED_MAPPINGS} )
add_dependencies( replicaTests dom flow)

CORAL_TARGET( replicaTests )

set_target_properties( replicaTests PROPERTIES
	PROJECT_LABEL "replicaTests"
	FOLDER "flowTests"
)

target_link_libraries( replicaTests ${SIVIEP_GTEST_LIBRARIES} ${CORAL_LIBRARIES} ${CALCIUM_LIBRARIES} ${_LIBRARIES} )

source_group( "@Generated" FILES ${_GENERATED_FILES} )

################################################################################
# Register the test
################################################################################

add_test(
		NAME replicaTests
		COMMAND $<TARGET_FILE:replicaTests> --gtest_output=xml:../output/replicaTests_$<CONFIGURATION>.xml
	)
	CORAL_TEST_ENVIRONMENT( replicaTests )
