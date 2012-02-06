################################################################################
# The 'tests' executable
################################################################################

PROJECT( TESTS )

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${TOTO_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )
FILE( GLOB _CORE_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp )
FILE( GLOB _NETWORK_HEADERS ${CMAKE_SOURCE_DIR}/src/network/*.h )
FILE( GLOB _NETWORK_SOURCES ${CMAKE_SOURCE_DIR}/src/network/*.cpp )

ADD_EXECUTABLE( tests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} ${_CORE_SOURCES} ${CMAKE_BINARY_DIR}/src/generated/__AllInOne.cpp ${_GENERATED_FILES} )
ADD_DEPENDENCIES( tests toto reef )

CORAL_TARGET( tests )

SET_TARGET_PROPERTIES( tests PROPERTIES
	PROJECT_LABEL "Tests"
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
			--log-file=${CMAKE_BINARY_DIR}/Valgrind$<CONFIGURATION>.log --error-exitcode=13
			--suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:tests>
	)
	CORAL_TEST_ENVIRONMENT( tests_MemoryCheck )
ENDIF()

################################################################################
# Source Groups
################################################################################

SOURCE_GROUP( "@Generated" FILES ${_GENERATED_FILES} )
