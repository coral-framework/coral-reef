################################################################################
# The 'rpcUnitTests' executable
################################################################################

project( RPCUNITTESTS )
CORAL_GENERATE_MODULE( _GENERATED_RPC_FILES rpc )
CORAL_GENERATE_MAPPINGS( _GENERATED_MAPPINGS "stubs.ITestSetup" "stubs.ISimpleTypes" 
"stubs.IComplexTypes" "stubs.IGrandParent" "stubs.IReferenceTypes" "stubs.IRefParent" 
"stubs.ISimpleParent" "stubs.IFakeLink" )

find_nuget ( siviep_protobuf )

include_directories(
	${SIVIEP_GTEST_INCLUDE_DIRS}
	${SIVIEP_PROTOBUF_INCLUDE_DIRS}
	${CMAKE_SOURCE_DIR}/src/rpc
	${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

#  sources replicated here for testing
file( GLOB _REPLICATED_MODULE_FILES	${CMAKE_SOURCE_DIR}/src/rpc/*.cpp ${CMAKE_SOURCE_DIR}/src/rpc/*.cc )

list( REMOVE_ITEM _GENERATED_RPC_FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/__Bootstrap.cpp" )

add_executable( rpcUnitTests EXCLUDE_FROM_ALL ${_REPLICATED_MODULE_FILES} ${_HEADERS} ${_SOURCES} ${_GENERATED_RPC_FILES} ${_GENERATED_MAPPINGS} )
add_dependencies( rpcUnitTests stubs )

CORAL_TARGET( rpcUnitTests )

set_target_properties( rpcUnitTests PROPERTIES
	PROJECT_LABEL "rpcUnitTests"
	FOLDER "rpcTests"
)

target_link_libraries( rpcUnitTests ${CORAL_LIBRARIES} ${SIVIEP_GTEST_LIBRARIES} ${SIVIEP_PROTOBUF_LIBRARIES} )

################################################################################
# Register the test
################################################################################
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/rpcUnitTests/output )
add_test(
	NAME rpcUnitTests
	COMMAND $<TARGET_FILE:rpcUnitTests> --gtest_output=xml:../../output/UnitTests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( rpcUnitTests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
if( VALGRIND_COMMAND )
	add_test(
		NAME rpcUnitTests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=30 --dsymutil=yes
			--gen-suppressions=no --log-file=${CMAKE_BINARY_DIR}/rpcUnitTestsValgrind$<CONFIGURATION>.log --error-exitcode=13
			--track-origins=yes --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:rpcUnitTests>
	)
	CORAL_TEST_ENVIRONMENT( rpcUnitTests_MemoryCheck )
endif()

################################################################################
# Source Groups
################################################################################
source_group( "@Replicated" FILES ${_REPLICATED_MODULE_FILES} ${_GENERATED_RPC_FILES} ${_GENERATED_MAPPINGS} )

