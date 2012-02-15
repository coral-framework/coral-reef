################################################################################
# The 'tests' executable
################################################################################

PROJECT( CLIENTTESTS )

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${TOTO_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )

ADD_EXECUTABLE( clientTests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_CORE_SOURCES} ${_GENERATED_FILES} )
ADD_DEPENDENCIES( clientTests toto reef )

CORAL_TARGET( clientTests )

SET_TARGET_PROPERTIES( clientTests PROPERTIES
	PROJECT_LABEL "clientTests"
)

TARGET_LINK_LIBRARIES( clientTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Register the test
################################################################################
FILE( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/clientTests/output )
ADD_TEST(
	NAME clientTests
	COMMAND $<TARGET_FILE:clientTests> --gtest_output=xml:../../output/Tests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( clientTests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
IF( VALGRIND_COMMAND )
	ADD_TEST(
		NAME tests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=30 --dsymutil=yes
			--log-file=${CMAKE_BINARY_DIR}/Valgrind$<CONFIGURATION>.log --error-exitcode=13
			--suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:tests>
	)
	CORAL_TEST_ENVIRONMENT( tests_MemoryCheck )
ENDIF()

################################################################################
# Source Groups
################################################################################

SOURCE_GROUP( "@Generated" FILES ${_GENERATED_FILES} )
