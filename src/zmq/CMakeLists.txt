################################################################################
# Build the Module
################################################################################
CORAL_GENERATE_MODULE( _GENERATED_FILES zmq )

include_directories( 
	${CMAKE_CURRENT_BINARY_DIR}/generated 
	${ZEROMQ_INCLUDE_DIR}
)

file( GLOB _SOURCES *.cpp )
file( GLOB _HEADERS *.h )

add_library( zmq MODULE ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
add_dependencies( zmq rpc )

CORAL_MODULE_TARGET( "zmq" zmq )

target_link_libraries( zmq ${CORAL_LIBRARIES} ${ZEROMQ_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
source_group( "Network" FILES ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS zmq
	DESTINATION "modules/zmq"
	COMPONENT "zmq Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/zmq/zmq_debug.pdb"
	DESTINATION "modules/zmq"
	CONFIGURATIONS Debug
	COMPONENT "zmq Module"
	OPTIONAL
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/zmq/zmq.pdb"
	DESTINATION "modules/zmq"
	CONFIGURATIONS RelWithDebInfo
	COMPONENT "zmq Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/zmq
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "zmq Module"
)
