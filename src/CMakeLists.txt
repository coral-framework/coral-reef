################################################################################
# Build the Module
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES reef lua.IState )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/generated )

FILE( GLOB _SOURCES *.cpp )
FILE( GLOB _NETWORK_SOURCES network/*.cpp )
FILE( GLOB _NETWORK_HEADERS network/*.h )
FILE( GLOB _HEADERS *.h )


ADD_LIBRARY( reef MODULE ${_HEADERS} ${_SOURCES} ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "reef" reef )

TARGET_LINK_LIBRARIES( reef ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

SOURCE_GROUP( "@Generated" FILES ${_GENERATED_FILES} )
SOURCE_GROUP( "Network" FILES ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS reef
	DESTINATION "modules/reef"
	COMPONENT "Reef Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/ca/ca_debug.pdb"
	DESTINATION "modules/ca"
	CONFIGURATIONS Debug
	COMPONENT "Reef Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/ca
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "Reef Module"
)
