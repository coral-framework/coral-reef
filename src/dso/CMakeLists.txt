################################################################################
# Build the Module RPC
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES dso "ca.IArchive" "ca.ISpacePersister" "ca.INamed" "lua.IState" )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _SOURCES *.cpp )
file( GLOB _HEADERS *.h )

add_library( dso MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
SET_TARGET_PROPERTIES( dso PROPERTIES
	PROJECT_LABEL "dso"
)

CORAL_MODULE_TARGET( "dso" dso )

add_dependencies( dso reef )

target_link_libraries( dso ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS dso
	DESTINATION "modules/dso"
	COMPONENT "Reef DSO Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/dso/dso_debug.pdb"
	DESTINATION "modules/dso"
	CONFIGURATIONS Debug
	COMPONENT "Reef DSO Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/dso
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "Reef DSO Module"
)
