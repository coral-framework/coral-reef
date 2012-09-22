#Use solution folders.
SET_PROPERTY( GLOBAL PROPERTY USE_FOLDERS ON )

add_subdirectory( rpc )

set( RPC_ONLY "OFF" CACHE BOOL "No flow module, rpc only" )
if( RPC_ONLY STREQUAL "OFF" )
	add_subdirectory( flow )
endif()

set( BUILD_ZMQ "OFF" CACHE BOOL "ZMQ transport layer" )
if( BUILD_ZMQ STREQUAL "ON" )
	message( "ZMQ is meant only for production, should not be used for testing and development" )
	add_subdirectory( zmq )
else()
	message( "ZMQ not built. Transport layer will be mock only" )
endif()