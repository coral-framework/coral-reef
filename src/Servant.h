#ifndef SERVANT_H
#define SERVANT_H

#include <co/IService.h>
#include <co/RefPtr.h>

namespace reef {

class Node;
struct Message;    

class Servant
{
public:
	Servant( co::IService* master, reef::Node* localNode, co::int32 rmtNodeId, co::int32 rmtProxyId );

	virtual ~Servant();

	void receiveMsg( Message* msg );

private:
	// member variables
	co::RefPtr<co::IService> _master;
	reef::Node* _localNode;
	co::int32 _rmtNodeId;
	co::int32 _rmtProxyId;
};

} // namespace reef


#endif