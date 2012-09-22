add_subdirectory( rpcIntegrationTests )
add_subdirectory( rpcUnitTests )
add_subdirectory( stubs )

ADD_CUSTOM_TARGET( rpcTests DEPENDS rpc stubs rpcUnitTests rpcIntegrationTests )

set_target_properties( rpcTests PROPERTIES
	PROJECT_LABEL "rpcTests"
	FOLDER "rpcTests"
)