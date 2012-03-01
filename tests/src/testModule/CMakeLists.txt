################################################################################
# testModule, an auxiliary test module
################################################################################

PROJECT( testModule )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES testModule )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/generated )

FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )

ADD_LIBRARY( testModule MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )

CORAL_MODULE_TARGET( "testModule" testModule )

SET_TARGET_PROPERTIES( testModule PROPERTIES
	PROJECT_LABEL "testModule"
)

TARGET_LINK_LIBRARIES( testModule ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

SOURCE_GROUP( "@Generated" FILES ${_GENERATED_FILES} )
