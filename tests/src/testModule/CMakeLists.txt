################################################################################
# testModule, an auxiliary test module
################################################################################

project( testModule )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES testModule )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( testModule MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "testModule" testModule )

set_target_properties( testModule PROPERTIES
	PROJECT_LABEL "testModule"
)

target_link_libraries( testModule ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
