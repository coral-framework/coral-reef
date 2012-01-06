################################################################################
# TOTO - Entity Relationship Model, an auxiliary test module
################################################################################

PROJECT( TOTO )

# Run the coral compiler on the module
CORAL_GENERATE_MODULE( _GENERATED_FILES toto )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR}/generated )

FILE( GLOB _HEADERS *.h )
FILE( GLOB _SOURCES *.cpp )

ADD_LIBRARY( toto MODULE EXCLUDE_FROM_ALL ${_HEADERS} ${_SOURCES} ${_GENERATED_FILES} )
ADD_DEPENDENCIES( toto ca )

CORAL_MODULE_TARGET( "toto" toto )

SET_TARGET_PROPERTIES( toto PROPERTIES
	PROJECT_LABEL "Module TOTO"
)

TARGET_LINK_LIBRARIES( toto ${CORAL_LIBRARIES} )

################################################################################
# Source Groups
################################################################################

SOURCE_GROUP( "@Generated" FILES ${_GENERATED_FILES} )
