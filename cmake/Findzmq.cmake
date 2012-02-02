# - Find the ZMQ library.
#
# It sets the following variables:
#  ZMQ_FOUND            - Set to false, or undefined, if gpos isn't found.
#  ZMQ_INCLUDE_DIR      - The ZMQ include directory.
#  ZMQ_LIBRARIES        - The ZMQ release libraries to link against.

file( TO_CMAKE_PATH "$ENV{ZMQ_ROOT}/" ZMQ_BASE_DIR )

find_path( ZMQ_INCLUDE_DIR zmq.h
	PATHS 
	${ZMQ_BASE_DIR}/include
)

find_library( ZMQ_LIBRARY
		  NAMES zmq
		  PATHS ${ZMQ_BASE_DIR}/lib
)
		
if( ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY )
	set( ZMQ_FOUND TRUE )
endif()

if( ZMQ_FOUND )
	# force resview to use its own versions of all external dependencies (self contained project)
	set( ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR} )
	set( ZMQ_LIBRARIES optimized ${ZMQ_LIBRARY} )
	if( NOT zmq_FIND_QUIETLY )
		message( STATUS "Found ZMQ ${zmq_FIND_VERSION}: ${ZMQ_LIBRARY}" )
	endif()
else()
	# fatal error if Resview is required but not found
	if( zmq_FIND_REQUIRED )
		message( FATAL_ERROR "Could not find ZMQ library." )
	endif()
endif()

