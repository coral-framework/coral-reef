################################################################################
# Build the Module RPC
################################################################################

find_nuget ( protobuf-vc100 2.5 )

CORAL_GENERATE_MODULE( _GENERATED_FILES rpc )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

## Generate Protocol buffer files
## TODO: protoc.exe should be inside protobuf's nuget, always updating both at the same time.
## TODO: Declare proto dependencies correctly so they are only generated when the proto files change.
file(GLOB_RECURSE _PROTOS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.proto) 

EXEC_PROGRAM(${CMAKE_SOURCE_DIR}/tools/protoc.exe ${CMAKE_CURRENT_SOURCE_DIR} 
	             ARGS --cpp_out="${CMAKE_CURRENT_SOURCE_DIR}" ${_PROTOS}
				 RETURN_VALUE _ERRORLEVEL)

IF(NOT ${_ERRORLEVEL} EQUAL 0)
	MESSAGE(FATAL_ERROR "Error compiling proto files.")
ENDIF()

file( GLOB _SOURCES *.cpp *.cc )
file( GLOB _HEADERS *.h )

add_library( rpc MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

SET_TARGET_PROPERTIES( rpc PROPERTIES
	PROJECT_LABEL "rpc"
)

CORAL_MODULE_TARGET( "rpc" rpc )

target_link_libraries( rpc ${CORAL_LIBRARIES} ${PROTOBUF-VC100_LIBRARIES})

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
source_group( "Network" FILES ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS rpc
	DESTINATION "modules/rpc"
	COMPONENT "RPC Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/rpc/rpc_debug.pdb"
	DESTINATION "modules/rpc"
	CONFIGURATIONS Debug
	COMPONENT "RPC Module"
	OPTIONAL
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/rpc/rpc.pdb"
	DESTINATION "modules/rpc"
	CONFIGURATIONS RelWithDebInfo
	COMPONENT "RPC Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/rpc
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "RPC Module"
)
