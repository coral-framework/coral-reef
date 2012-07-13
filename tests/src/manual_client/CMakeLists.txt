################################################################################
# The 'tests' executable
################################################################################

project( CLIENTTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${dom_BINARY_DIR}/generated
	${rpcTests_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/reef/rpc/generated
	${CMAKE_SOURCE_DIR}/src/reef/rpc
	${CMAKE_BINARY_DIR}/src/dso/generated
	${CMAKE_SOURCE_DIR}/src/dso
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( manual_client EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} )
add_dependencies( manual_client moduleA reef dso dom )

CORAL_TARGET( manual_client )

set_target_properties( manual_client PROPERTIES
	PROJECT_LABEL "manual_client"
)

target_link_libraries( manual_client ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )
