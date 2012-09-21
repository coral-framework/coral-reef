################################################################################
# Build the Module RPC
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES flow "ca.IArchive" "ca.ISpacePersister" "ca.INamed" "lua.IState" )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _SOURCES *.cpp )
file( GLOB _HEADERS *.h )

add_library( flow MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
SET_TARGET_PROPERTIES( flow PROPERTIES
	PROJECT_LABEL "flow"
)

CORAL_MODULE_TARGET( "flow" flow )

add_dependencies( flow rpc )

target_link_libraries( flow ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS flow
	DESTINATION "modules/flow"
	COMPONENT "FLOW Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/flow/flow_debug.pdb"
	DESTINATION "modules/flow"
	CONFIGURATIONS Debug
	COMPONENT "FLOW Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/flow
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "FLOW Module"
)
