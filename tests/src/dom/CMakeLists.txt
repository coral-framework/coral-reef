################################################################################
# DOM, an auxiliary test module
################################################################################

project( dom )
# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES dom )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( dom MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "dom" dom )

set_target_properties( dom PROPERTIES
	PROJECT_LABEL "dom"
)

target_link_libraries( dom ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
