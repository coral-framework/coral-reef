# - Find the ZMQ
# This module accepts the following environment variables:
# ZMQ_ROOT or ZMQ_DIR or ZEROMQ_ROOT - Specify the location of ZMQ
#
# This module defines the following variables:
# ZMQ_INCLUDE_DIRS		- include directories for ZMQ
# ZMQ_LIBRARIES 		- libraries to link against ZMQ
# ZMQ_FOUND 			- true if ZMQ has been found and can be used

#=============================================================================
# Copyright 2014 SiVIEP
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# Looking for the include files
FIND_PATH( ZMQ_INCLUDE_DIR 
	NAMES
		zmq.h
	HINTS 
		ENV ZMQ_ROOT
		ENV ZMQ_DIR
		ENV ZEROMQ_ROOT
	PATH_SUFFIXES 
		include 
)

# Looking for release library
# Also called XXX_LIBRARY_RELEASE
FIND_LIBRARY( ZMQ_LIBRARY
	NAMES 
		zmq zmq_r zmq_release zdll libzmq-v120-mt-4_0_4 libzmq
	HINTS
		ENV ZMQ_ROOT
		ENV ZMQ_DIR
		ENV ZEROMQ_ROOT
	PATH_SUFFIXES
		lib/release
		lib
)

# Looking for debug library
FIND_LIBRARY( ZMQ_LIBRARY_DEBUG
	NAMES 
		zmqd zmq_d zmq_debug zdlld libzmq-v120-mt-gd-4_0_4 libzmq libzmqd
	HINTS
		ENV ZMQ_ROOT
		ENV ZMQ_DIR
		ENV ZEROMQ_ROOT
	PATH_SUFFIXES
		lib/debug
		lib
)

# Handle the QUIETLY and REQUIRED arguments and set ZMQ_FOUND to TRUE if all listed variables are TRUE
FIND_PACKAGE( PackageHandleStandardArgs REQUIRED )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( ZMQ REQUIRED_VARS ZMQ_INCLUDE_DIR ZMQ_LIBRARY )

if( ZMQ_FOUND )
	SET( ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR} )
	
	# Merging release and debug libraries
	# IMPORTANT: "debug" and "optimized" keywords must be lowercase
	SET( ZMQ_LIBRARIES 
		optimized ${ZMQ_LIBRARY} 
		debug ${ZMQ_LIBRARY_DEBUG}
	)
endif()

MARK_AS_ADVANCED( 
	ZMQ_INCLUDE_DIR
	ZMQ_LIBRARY
	ZMQ_LIBRARY_DEBUG
)
