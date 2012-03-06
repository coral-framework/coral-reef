#ifndef _REEF_FAKESOCKET_
#define _REEF_FAKESOCKET_

#include <map>
#include <string>

namespace reef
{

class Binder;
class BinderQueue;

class ReplyDelegate
{
public:
	virtual void replyableMsgSent() const = 0;
};

class FakeSocket
{
public:
	// This is necessary as the msgs that trigger reply will block. Thus, the repDel 
	// will be notified and will post the reply before the sendAt msg returns
	static void setReplyDelegateAt( const ReplyDelegate* repDel, const std::string& address );

	static void sendAt( const std::string& msg, const std::string& binderAddress );

	static void receiveAt( std::string& msg, const std::string& binderAddress );

	static void reply( const std::string& msg, const std::string& binderAddress );

	static void receiveReply( std::string& msg, const std::string& binderAddress );

	static void bindOrConnectAt( const std::string& address );

	static void closeAt( const std::string& address );

private:

	// convenience functions to retrieve values from a map
	static BinderQueue* getQueueAt( const std::string& address );
};

}

#endif