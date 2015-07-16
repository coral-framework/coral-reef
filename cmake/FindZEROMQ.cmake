SET( LIBRARY_NAME ZEROMQ )

# --------------------------------------------------------------------
# Find the ZEROMQ library
# This module accepts the following environment variables:
# ZEROMQ_ROOT or ZEROMQ_DIR - Specify the location of ZEROMQ
#
# This module defines the following variables:
# ZEROMQ_INCLUDE_DIR  	- include directory for ZEROMQ
# ZEROMQ_LIBRARIES	 	- libraries to link against ZEROMQ
# ZEROMQ_FOUND		 	- true if ZEROMQ has been found and can be used
# --------------------------------------------------------------------

# Looking for the include files
FIND_PATH( ${LIBRARY_NAME}_INCLUDE_DIR 
	NAMES 
		zmq.h
	HINTS
		ENV ${LIBRARY_NAME}_ROOT
		ENV ${LIBRARY_NAME}_DIR
	PATH_SUFFIXES
		include
)

# Looking for release library
# Also called ZEROMQ_LIBRARY_RELEASE
FIND_LIBRARY( ${LIBRARY_NAME}_LIBRARY
	NAMES 
		libzmq
	HINTS
		ENV ${LIBRARY_NAME}_ROOT
		ENV ${LIBRARY_NAME}_DIR
	PATH_SUFFIXES
		lib/release
		lib
)

# Looking for debug library
FIND_LIBRARY( ${LIBRARY_NAME}_LIBRARY_DEBUG
	NAMES 
		libzmqd
	HINTS
		ENV ${LIBRARY_NAME}_ROOT
		ENV ${LIBRARY_NAME}_DIR
	PATH_SUFFIXES
		lib/debug
		lib
)

# Handle the QUIETLY and REQUIRED arguments and set ZEROMQ_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE( PackageHandleStandardArgs REQUIRED )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( ${LIBRARY_NAME} REQUIRED_VARS ${LIBRARY_NAME}_INCLUDE_DIR ${LIBRARY_NAME}_LIBRARY )

IF( ${LIBRARY_NAME}_FOUND )
	IF( ${LIBRARY_NAME}_LIBRARY_DEBUG )
		# Merging release and debug libraries
		# IMPORTANT: "debug" and "optimized" keywords must be lowercase
		SET( ${LIBRARY_NAME}_LIBRARIES optimized ${${LIBRARY_NAME}_LIBRARY} debug ${${LIBRARY_NAME}_LIBRARY_DEBUG} )	
	ELSE()
		SET( ${LIBRARY_NAME}_LIBRARIES ${${LIBRARY_NAME}_LIBRARY} )
	ENDIF()
ENDIF()

MARK_AS_ADVANCED( 
	${LIBRARY_NAME}_INCLUDE_DIR
	${LIBRARY_NAME}_LIBRARY
	${LIBRARY_NAME}_LIBRARY_DEBUG
)