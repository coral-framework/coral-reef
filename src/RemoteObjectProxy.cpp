/*
 * Component implementation template for 'reef.RemoteObjectProxy'.
 */

#include "RemoteObjectProxy_Base.h"

namespace reef {

class RemoteObjectProxy : public RemoteObjectProxy_Base
{
public:
	RemoteObjectProxy()
	{
		// empty constructor
	}

	virtual ~RemoteObjectProxy()
	{
		// empty destructor
	}

	// ------ reef.IRemoteObjectProxy Methods ------ //

	const std::string& getMethods()
	{
		static std::string dummy;
		return dummy;
	}

	void setRemoteObjectId( co::int32 id )
	{
		// TODO: implement this method.
	}

private:
	// member variables
};

CORAL_EXPORT_COMPONENT( RemoteObjectProxy, RemoteObjectProxy );

} // namespace reef
