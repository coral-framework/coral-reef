/*
 * Component implementation template for 'reef.Servant'.
 */
#include "Servant.h"
#include <co/IMethod.h>
#include <co/IReflector.h>

#include "Networking.h"
#include "Node.h"

#include <string.h>
#include <stdio.h>

namespace reef {

Servant::Servant( co::IService* master, reef::Node* node, co::int32 channel ) : 
	 _master( master ), _channel( channel ), _node( node )
{
	// empty constructor
}

Servant::~Servant()
{
	// empty destructor
}

void Servant::receiveMsg( Message& msg )
{
	void* data = NULL;
	memcpy( data, msg.buffer, msg.bytes );

	// for now only member requests will be attended
    co::IInterface* reqInterface = _master->getInterface();
	co::IMember* reqMember;

	if( ((char*)data)[0] == 's' )
	{
		std::string memberName(((char*)data) + 1); 
		printf( "Member called %s requested\n", ((char*)data) + 1  );
		reqMember = reqInterface->getMember( memberName );
	}
	else if( ((char*)data)[0] == 'n' )
	{
		int* numericData = (int*)( ((char*)data) + 1 );
		reqMember = reqInterface->getMembers()[numericData[0]];
	}

	co::MemberKind kind = reqMember->getKind();
	
	if( kind == co::MemberKind::MK_METHOD )
	{
		// call method. for now, no returning supported. When returning, fall into same cases of field getters.
		co::IReflector* reflector( reqInterface->getReflector() );
		co::Any retValue;
		
		co::IMethod* reqMethod( co::cast<co::IMethod>( reqMember ) );
		reflector->invoke( _master.get(), reqMethod, co::Range<co::Any const>(), retValue );
	}
	/* else if( kind == co::MemberKind::MK_FIELD )
		// in case of value type, simply return it. in case of ref type request another proxy creation
	else if( kind == co::MemberKind::MK_PORT )
		// not supported yet. When supported, will need another proxy creation.*/
}

} // namespace reef
