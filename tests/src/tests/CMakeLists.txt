################################################################################
# The 'tests' executable
################################################################################

project( TESTS )
CORAL_GENERATE_MODULE( _GENERATED_FILES reef )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${testModule_BINARY_DIR}/generated
	${REEF_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

# Calcium sources replicated here for testing
file( GLOB _REPLICATED_MODULE_FILES	${CMAKE_SOURCE_DIR}/src/*.cpp )

add_executable( tests EXCLUDE_FROM_ALL ${_REPLICATED_MODULE_FILES} ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
add_dependencies( tests testModule )

CORAL_TARGET( tests )

set_target_properties( tests PROPERTIES
	PROJECT_LABEL "tests"
)

target_link_libraries( tests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Register the test
################################################################################
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/output )
add_test(
	NAME tests
	COMMAND $<TARGET_FILE:tests> --gtest_output=xml:../../output/Tests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( tests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
if( VALGRIND_COMMAND )
	add_test(
		NAME tests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=30 --dsymutil=yes
			--log-file=${CMAKE_BINARY_DIR}/Valgrind$<CONFIGURATION>.log --error-exitcode=13
			--suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:tests>
	)
	CORAL_TEST_ENVIRONMENT( tests_MemoryCheck )
endif()

################################################################################
# Source Groups
################################################################################
source_group( "@Replicated" FILES ${_REPLICATED_MODULE_FILES} ${_GENERATED_FILES} )

