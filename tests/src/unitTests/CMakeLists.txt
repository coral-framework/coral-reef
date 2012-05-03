################################################################################
# The 'unitTests' executable
################################################################################

project( UNITTESTS )
CORAL_GENERATE_MODULE( _GENERATED_REEF_FILES reef )
CORAL_GENERATE_MAPPINGS( _GENERATED_MAPPINGS "mockReef.IFakeLink" )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${REEF_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/generated
	${moduleA_BINARY_DIR}/generated
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

# Calcium sources replicated here for testing
file( GLOB _REPLICATED_MODULE_FILES	${CMAKE_SOURCE_DIR}/src/*.cpp )
list( REMOVE_ITEM _GENERATED_REEF_FILES "${CMAKE_CURRENT_BINARY_DIR}/generated/__Bootstrap.cpp" )

add_executable( unitTests EXCLUDE_FROM_ALL ${_REPLICATED_MODULE_FILES} ${_HEADERS} ${_SOURCES} ${_GENERATED_REEF_FILES} ${_GENERATED_MAPPINGS} )
add_dependencies( unitTests moduleA mockReef )

CORAL_TARGET( unitTests )

set_target_properties( unitTests PROPERTIES
	PROJECT_LABEL "unitTests"
)

target_link_libraries( unitTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Register the test
################################################################################
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/unitTests/output )
add_test(
	NAME unitTests
	COMMAND $<TARGET_FILE:unitTests> --gtest_output=xml:../../output/UnitTests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( unitTests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
if( VALGRIND_COMMAND )
	add_test(
		NAME unitTests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=5 --dsymutil=yes
			--gen-suppressions=no --log-file=${CMAKE_BINARY_DIR}/unitTestsValgrind$<CONFIGURATION>.log --error-exitcode=13
			--track-origins=yes --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:unitTests>
	)
	CORAL_TEST_ENVIRONMENT( unitTests_MemoryCheck )
endif()

################################################################################
# Source Groups
################################################################################
source_group( "@Replicated" FILES ${_REPLICATED_MODULE_FILES} ${_GENERATED_REEF_FILES} ${_GENERATED_MAPPINGS} )

