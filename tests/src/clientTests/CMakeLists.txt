################################################################################
# The 'tests' executable
################################################################################

project( CLIENTTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${testModule_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( clientTests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} )
add_dependencies( clientTests testModule reef )

CORAL_TARGET( clientTests )

set_target_properties( clientTests PROPERTIES
	PROJECT_LABEL "clientTests"
)

target_link_libraries( clientTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} )

