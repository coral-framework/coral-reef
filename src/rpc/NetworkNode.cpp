#include "NetworkNode_Base.h"

namespace rpc {

class NetworkNode : public NetworkNode_Base
{
public:
	NetworkNode()
	{
		// empty constructor
	}

	virtual ~NetworkNode()
	{
		// empty destructor
	}

	//------ rpc.INetworkNode Methods ------//

	std::string getAddress()
	{
		return _address;
	}

	co::int32 getPort()
	{
		return _port;
	}

	void setPort( co::int32 port )
	{
		_port = port;
	}

	void setAddress(const std::string& addr)
	{
		_address = addr;
	}

private:
	// member variables
	std::string _address;
	co::int32 _port;
};

CORAL_EXPORT_COMPONENT( NetworkNode, NetworkNode );

} // namespace rpc
