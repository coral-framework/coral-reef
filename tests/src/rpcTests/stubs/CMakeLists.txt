################################################################################
# stubs, an auxiliary test module
################################################################################

project( stubs )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES stubs )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/generated )

file( GLOB _HEADERS *.h )
file( GLOB _SOURCES *.cpp )

add_library( stubs MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "stubs" stubs )

set_target_properties( stubs PROPERTIES
	PROJECT_LABEL "stubs"
	FOLDER "rpcTests"
)

target_link_libraries( stubs ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

source_group( "@Generated" FILES ${_GENERATED_FILES} )
