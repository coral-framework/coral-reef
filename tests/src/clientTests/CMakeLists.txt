################################################################################
# The 'tests' executable
################################################################################

PROJECT( CLIENTTESTS )

INCLUDE_DIRECTORIES(
	${GTEST_INCLUDE_DIRS}
	${testModule_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )

ADD_EXECUTABLE( clientTests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} )
ADD_DEPENDENCIES( clientTests testModule reef )

CORAL_TARGET( clientTests )

SET_TARGET_PROPERTIES( clientTests PROPERTIES
	PROJECT_LABEL "clientTests"
)

TARGET_LINK_LIBRARIES( clientTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} )

