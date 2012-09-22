################################################################################
# The 'tests' executable
################################################################################

project( SERVERTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${dom_BINARY_DIR}/generated
	${stubs_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/rpc/generated
	${CMAKE_SOURCE_DIR}/src/rpc
	${CMAKE_BINARY_DIR}/src/flow/generated
	${CMAKE_SOURCE_DIR}/src/flow
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( manual_server EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} )
add_dependencies( manual_server stubs rpc )

CORAL_TARGET( manual_server )

set_target_properties( manual_server PROPERTIES
	PROJECT_LABEL "manual_server"
	FOLDER "manualTests"
)

target_link_libraries( manual_server ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )
