################################################################################
# moduleA, an auxiliary test module
################################################################################

project( moduleA )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES moduleA )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( moduleA MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "moduleA" moduleA )

set_target_properties( moduleA PROPERTIES
	PROJECT_LABEL "moduleA"
)

target_link_libraries( moduleA ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
