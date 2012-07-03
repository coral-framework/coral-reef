################################################################################
# The 'tests' executable
################################################################################

project( SERVERTESTS )

include_directories(
	${GTEST_INCLUDE_DIRS}
	${rpcTests_BINARY_DIR}/generated
	${CMAKE_BINARY_DIR}/src/reef/rpc/generated
	${CMAKE_SOURCE_DIR}/src/reef/rpc
)

# Gather source files in the current directory
file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_executable( manual_server EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_CORE_SOURCES} ${_GENERATED_FILES} )
add_dependencies( manual_server rpcTests reef )

CORAL_TARGET( manual_server )

set_target_properties( manual_server PROPERTIES
	PROJECT_LABEL "manual_server"
)

target_link_libraries( manual_server ${CORAL_LIBRARIES} ${GTEST_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
