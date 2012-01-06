/*
 * Component implementation template for 'reef.RemoteServiceProxy'.
 */

#include "RemoteServiceProxy_Base.h"
#include "Message.h"

namespace reef {

class RemoteServiceProxy : public RemoteServiceProxy_Base
{
public:
	RemoteServiceProxy()
	{
		// empty constructor
	}

	virtual ~RemoteServiceProxy()
	{
		// empty destructor
	}

	void setServiceId( co::int32 id )
	{
		serviceId = id;
	}

private:

	co::int32 serviceId;
};

CORAL_EXPORT_COMPONENT( RemoteServiceProxy, RemoteServiceProxy );

} // namespace reef
