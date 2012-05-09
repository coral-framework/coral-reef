################################################################################
# Build the Module RPC
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES reef.rpc )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _SOURCES *.cpp )
file( GLOB _HEADERS *.h )

add_library( rpc MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
SET_TARGET_PROPERTIES( rpc PROPERTIES
	PROJECT_LABEL "rpc"
	FOLDER "reef"
)

CORAL_MODULE_TARGET( "reef.rpc" rpc )

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
	DESTINATION "modules/reef/rpc"
	COMPONENT "Reef RPC Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/reef/rpc/rpc_debug.pdb"
	DESTINATION "modules/reef"
	CONFIGURATIONS Debug
	COMPONENT "Reef RPC Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/reef/rpc
	DESTINATION "modules/reef"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "Reef RPC Module"
)
