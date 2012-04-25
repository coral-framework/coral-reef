################################################################################
# The 'tests' executable
################################################################################

project( SERVERTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${CMAKE_BINARY_DIR}/src/generated
	${CMAKE_SOURCE_DIR}/src
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( serverTests EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_CORE_SOURCES} ${_GENERATED_FILES} )
add_dependencies( serverTests moduleA reef )

CORAL_TARGET( serverTests )

set_target_properties( serverTests PROPERTIES
	PROJECT_LABEL "serverTests"
)

target_link_libraries( serverTests ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
