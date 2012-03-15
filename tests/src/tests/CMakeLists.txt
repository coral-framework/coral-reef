################################################################################
# The 'tests' executable
################################################################################

PROJECT( TESTS )
CORAL_GENERATE_MODULE( _GENERATED_FILES reef )

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${testModule_BINARY_DIR}/generated
	${REEF_SOURCE_DIR}/src
	${CMAKE_CURRENT_BINARY_DIR}/generated
)

# Gather source files in the current directory
FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )

# Calcium sources replicated here for testing
FILE( GLOB _REPLICATED_MODULE_FILES	${CMAKE_SOURCE_DIR}/src/*.cpp )

ADD_EXECUTABLE( tests EXCLUDE_FROM_ALL ${_REPLICATED_MODULE_FILES} ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
ADD_DEPENDENCIES( tests testModule )

CORAL_TARGET( tests )

SET_TARGET_PROPERTIES( tests PROPERTIES
	PROJECT_LABEL "tests"
)

TARGET_LINK_LIBRARIES( tests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Register the test
################################################################################
FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests/output )
ADD_TEST(
	NAME tests
	COMMAND $<TARGET_FILE:tests> --gtest_output=xml:../../output/Tests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( tests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
IF( VALGRIND_COMMAND )
	ADD_TEST(
		NAME tests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=30 --dsymutil=yes
			--gen-suppressions=all --log-file=${CMAKE_BINARY_DIR}/Valgrind$<CONFIGURATION>.log --error-exitcode=13
			--suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:tests>
	)
	CORAL_TEST_ENVIRONMENT( tests_MemoryCheck )
ENDIF()

################################################################################
# Source Groups
################################################################################
SOURCE_GROUP( "@Replicated" FILES ${_REPLICATED_MODULE_FILES} ${_GENERATED_FILES} )

