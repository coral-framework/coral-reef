################################################################################
# Build the Module
################################################################################

CORAL_GENERATE_MODULE( _GENERATED_FILES reef )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _SOURCES *.cpp )
file( GLOB _NETWORK_SOURCES network/*.cpp )
file( GLOB _NETWORK_HEADERS network/*.h )
file( GLOB _HEADERS *.h )


add_library( reef MODULE ${_HEADERS} ${_SOURCES} ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "reef" reef )

target_link_libraries( reef ${CORAL_LIBRARIES} ${ZMQ_LIBRARIES} ${PROTOBUF_LIBRARIES} ${PROTOBUF_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARY_DEBUG} ${PROTOBUF_PROTOC_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
source_group( "Network" FILES ${_NETWORK_SOURCES} ${_NETWORK_HEADERS} )

################################################################################
# Install Rules
################################################################################

# install shared library
INSTALL( TARGETS reef
	DESTINATION "modules/reef"
	COMPONENT "Reef Module"
)

INSTALL( FILES "${CMAKE_BINARY_DIR}/modules/reef/reef_debug.pdb"
	DESTINATION "modules/reef"
	CONFIGURATIONS Debug
	COMPONENT "Reef Module"
	OPTIONAL
)

# install module files
INSTALL( DIRECTORY ${CMAKE_SOURCE_DIR}/modules/reef
	DESTINATION "modules"
	CONFIGURATIONS Release RelWithDebInfo
	COMPONENT "Reef Module"
)
