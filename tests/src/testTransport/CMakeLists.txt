################################################################################
# testTransport, an auxiliary test module
################################################################################

project( testTransport )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES testTransport )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( testTransport MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "testTransport" testTransport )

set_target_properties( testTransport PROPERTIES
	PROJECT_LABEL "testTransport"
)

target_link_libraries( testTransport ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
