################################################################################
# rpcTests, an auxiliary test module
################################################################################

project( rpcTests )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES rpcTests )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( rpcTests MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "rpcTests" rpcTests )

set_target_properties( rpcTests PROPERTIES
	PROJECT_LABEL "rpcTests"
)

target_link_libraries( rpcTests ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
