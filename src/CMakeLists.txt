#Use solution folders.
SET_PROPERTY( GLOBAL PROPERTY USE_FOLDERS ON )

add_subdirectory( reef )

set( BUILD_ZMQ ON CACHE BOOL "Building ZMQ also" )
if( BUILD_ZMQ )
	message( "ZMQ is meant only for production, should not be used for testing and development" )
	add_subdirectory( zmq )
else()
	message( "ZMQ not built. Transport layer will be mock only" )
endif()