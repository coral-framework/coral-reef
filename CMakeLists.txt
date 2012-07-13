################################################################################
# Reef - Networking Module
################################################################################

cmake_minimum_required( VERSION 2.6 )

project( REEF )

################################################################################
# Setup Coral
################################################################################

# Load Coral's CMake package
if( NOT CORAL_ROOT )
	file( TO_CMAKE_PATH "$ENV{CORAL_ROOT}" CORAL_ROOT )
endif()
set( CMAKE_MODULE_PATH ${CORAL_ROOT}/cmake ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )
find_package( Coral REQUIRED )

set( CORAL_PATH
	"${CMAKE_BINARY_DIR}/modules"
	"${CMAKE_SOURCE_DIR}/modules"
	${CORAL_PATH}
)

# FindPROTOBUF.cmake looks for PROTOBUF_SRC_ROOT_FOLDER cmake variable instead of an ENV var
if( WIN32 )
	file( TO_CMAKE_PATH "$ENV{PROTOBUF_ROOT}" PROTOBUF_SRC_ROOT_FOLDER )
	set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/optional ${CMAKE_MODULE_PATH} )
endif()

find_package( Protobuf REQUIRED )

include_directories( ${CORAL_INCLUDE_DIRS} ${ZMQ_INCLUDE_DIRS} ${PROTOBUF_INCLUDE_DIRS} )

################################################################################
# Installation
################################################################################

# enforce a safe default CMAKE_INSTALL_PREFIX
if( NOT CMAKE_INSTALL_PREFIX MATCHES "coral-reef" )
	set( CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/coral-reef" )
endif()

################################################################################
# Packaging
################################################################################

set( CPACK_PACKAGE_NAME					"coral-reef" )
set( CPACK_PACKAGE_VERSION_MAJOR		"0" )
set( CPACK_PACKAGE_VERSION_MINOR		"1" )
set( CPACK_PACKAGE_VERSION_PATCH		"0" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY	"Reef is a networking module that provides an API to search/access remote objects (component instantiation)" )

if( WIN32 )
	set( CPACK_GENERATOR "ZIP" )
else()
	set( CPACK_GENERATOR "TBZ2" )
endif()

include( CPack )

################################################################################
# Subdirectories
################################################################################

add_subdirectory( src )

ENABLE_TESTING()
add_subdirectory( tests )
