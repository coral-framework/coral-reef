/*
 * Component implementation template for 'rpc.NetworkNode'.
 * WARNING: remember to copy this file to your project dir before you begin to change it.
 * Generated by the Coral Compiler v0.8.1 on 2015-08-21 14:00:35.p
 */

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

private:
	// member variables
	std::string _address;
	co::int32 _port;
};

CORAL_EXPORT_COMPONENT( NetworkNode, NetworkNode );

} // namespace rpc
