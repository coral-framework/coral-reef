add_subdirectory( replicaTests )
add_subdirectory( dom )

ADD_CUSTOM_TARGET( flowTests DEPENDS rpc flow dom replicaTests )

set_target_properties( flowTests PROPERTIES
	PROJECT_LABEL "flowTests"
	FOLDER "flowTests"
)