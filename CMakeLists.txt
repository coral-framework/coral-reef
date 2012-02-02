################################################################################
# Reef - Networking Module
################################################################################

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

PROJECT( REEF )

################################################################################
# Setup Coral
################################################################################

# Resolve the CORAL_ROOT
FILE( TO_CMAKE_PATH "$ENV{CORAL_ROOT}" CORAL_ROOT )

# Add Coral's CMake dir to the CMAKE_MODULE_PATH
SET( CMAKE_MODULE_PATH ${CORAL_ROOT}/cmake ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

# Load Coral's CMake package
FIND_PACKAGE( Coral REQUIRED )
FIND_PACKAGE( zmq REQUIRED )

# Configure the coral path
SET( CORAL_PATH
	"${CORAL_ROOT}/modules"
	"${CMAKE_BINARY_DIR}/modules"
	"${CMAKE_SOURCE_DIR}/modules"
)

INCLUDE_DIRECTORIES( ${CORAL_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS} )

################################################################################
# Installation
################################################################################

# enforce a safe default CMAKE_INSTALL_PREFIX
IF( NOT CMAKE_INSTALL_PREFIX MATCHES "coral-reef" )
	SET( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/coral-reef" )
ENDIF()

################################################################################
# Packaging
################################################################################

SET( CPACK_PACKAGE_NAME					"coral-reef" )
SET( CPACK_PACKAGE_VERSION_MAJOR		"0" )
SET( CPACK_PACKAGE_VERSION_MINOR		"1" )
SET( CPACK_PACKAGE_VERSION_PATCH		"0" )
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Reef is a networking module that provides an API to search/access remote objects (component instantiation)" )

IF( WIN32 )
	SET( CPACK_GENERATOR "ZIP" )
ELSE()
	SET( CPACK_GENERATOR "TBZ2" )
ENDIF()

INCLUDE( CPack )

################################################################################
# Subdirectories
################################################################################

ADD_SUBDIRECTORY( src )

ENABLE_TESTING()
ADD_SUBDIRECTORY( tests )
