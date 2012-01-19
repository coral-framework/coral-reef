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

Servant::Servant( co::IService* master, reef::Node* localNode, co::int32 rmtNodeId, co::int32 rmtProxyId ) : 
	 _master( master ), _localNode( localNode ), _rmtNodeId( rmtNodeId ), _rmtProxyId( rmtProxyId )
{
	// empty constructor
}

Servant::~Servant()
{
	// empty destructor
}

void Servant::receiveMsg( Message* msg )
{
	// for now only member requests will be attended
    co::IInterface* reqInterface = _master->getInterface();
	co::IMember* reqMember;

	if( msg->type == 's' )
	{
        std::string memberName(  static_cast<char*>( msg->data ) );
		printf( "Member called %s requested\n", memberName.c_str() );
		reqMember = reqInterface->getMember( memberName );
	}
	else if( msg->type == 'n' )
	{
        co::int32 memberId = *static_cast<co::int32*>( msg->data );
		reqMember = reqInterface->getMembers()[memberId];
	}
    else
    {
        return;
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
