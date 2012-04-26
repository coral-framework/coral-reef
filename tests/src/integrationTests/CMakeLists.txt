################################################################################
# The 'integrationTests' executable
################################################################################

project( INTEGRATIONTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${moduleA_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( integrationTests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} )
add_dependencies( integrationTests moduleA reef )

CORAL_TARGET( integrationTests )

set_target_properties( integrationTests PROPERTIES
	PROJECT_LABEL "integrationTests"
)

target_link_libraries( integrationTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} )

################################################################################
# Register the test
################################################################################
file( MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/integrationTests/output )
add_test(
	NAME integrationTests
	COMMAND $<TARGET_FILE:integrationTests> --gtest_output=xml:../../output/IntegrationTests$<CONFIGURATION>.xml
)
CORAL_TEST_ENVIRONMENT( integrationTests )

################################################################################
# If Valgrind is available, repeat the test checking for memory leaks
################################################################################
if( VALGRIND_COMMAND )
	add_test(
		NAME integrationTests_MemoryCheck
		COMMAND ${VALGRIND_COMMAND} --leak-check=full --show-reachable=yes --num-callers=5 --dsymutil=yes
			--gen-suppressions=no --log-file=${CMAKE_BINARY_DIR}/integrationTestsValgrind$<CONFIGURATION>.log --error-exitcode=13
			--track-origins=yes --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind.supp $<TARGET_FILE:integrationTests>
	)
	CORAL_TEST_ENVIRONMENT( integrationTests_MemoryCheck )
endif()