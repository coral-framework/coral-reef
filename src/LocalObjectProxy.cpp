/*
 * Component implementation template for 'reef.LocalServiceProxy'.
 */

#include "LocalServiceProxy_Base.h"
#include <co/RefPtr.h>
#include <co/IMethod.h>
#include <co/IReflector.h>

#include "Message.h"

#include <string.h>
#include <stdio.h>

namespace reef {

class LocalServiceProxy : public LocalServiceProxy_Base, public MsgObserver
{
public:
	LocalServiceProxy()
	{
		// empty constructor
	}

	virtual ~LocalServiceProxy()
	{
		// empty destructor
	}

	void msgReceived( Message msg )
	{
		void* data = NULL;
		memcpy( data, msg.buffer, msg.bytes );

		// for now only member requests will be attended
		co::IMember* reqMember;

		if( ((char*)data)[0] == 's' )
		{
			std::string memberName(((char*)data) + 1); 
			printf( "Member called %s requested\n", ((char*)data) + 1  );
			reqMember = _reqInterface->getMember( memberName );
		}
		else if( ((char*)data)[0] == 'n' )
		{
			int* numericData = (int*)( ((char*)data) + 1 );
			reqMember = _reqInterface->getMembers()[numericData[0]];
		}

		co::MemberKind kind = reqMember->getKind();
		
		if( kind == co::MemberKind::MK_METHOD )
		{
			// call method. for now, no returning supported. When returning, fall into same cases of field getters.
			co::IReflector* reflector( _reqInterface->getReflector() );
			co::Any retValue;
			
			co::IMethod* reqMethod( co::cast<co::IMethod>( reqMember ) );
			reflector->invoke( _reqService.get(), reqMethod, co::Range<co::Any const>(), retValue );
		}
		/* else if( kind == co::MemberKind::MK_FIELD )
			// in case of value type, simply return it. in case of ref type request another proxy creation
		else if( kind == co::MemberKind::MK_PORT )
			// not supported yet. When supported, will need another proxy creation.*/
	}

	// ------ reef.ILocalServiceProxy Methods ------ //

	void setServiceId( co::int32 id )
	{
		serviceId = id;
		Receiver::addMsgObserver( id, this );
	}

protected:
	// ------ Receptacle 'object' (co.IService) ------ //

	co::IService* getServiceService()
	{
		return _reqService.get();
	}

	void setServiceService( co::IService* service )
	{
		_reqService = service;
		_reqInterface = _reqService->getInterface();
	}

private:
	// member variables
	co::RefPtr<co::IService> _reqService;
	co::IInterface* _reqInterface;


	co::int32 serviceId;
};

CORAL_EXPORT_COMPONENT( LocalServiceProxy, LocalServiceProxy );

} // namespace reef
