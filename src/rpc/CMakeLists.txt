################################################################################
# Build the Module RPC
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES rpc )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _SOURCES *.cpp )
file( GLOB _HEADERS *.h )

add_library( rpc MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
SET_TARGET_PROPERTIES( rpc PROPERTIES
	PROJECT_LABEL "rpc"
)

CORAL_MODULE_TARGET( "rpc" rpc )

target_link_libraries( rpc ${CORAL_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} )

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

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/rpc
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "RPC Module"
)
