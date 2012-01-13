#ifndef SERVANT_H
#define SERVANT_H

#include <co/IService.h>
#include <co/RefPtr.h>

namespace reef {

class Node;
class Message;    

class Servant
{
public:
	Servant( co::IService* master, reef::Node* node, co::int32 channel );

	virtual ~Servant();

	void receiveMsg( Message& msg );

private:
	// member variables
	co::RefPtr<co::IService> _master;
	co::int32 _channel;
	reef::Node* _node;
};

} // namespace reef


#endif