#ifndef MESSAGE_H

struct Message
{
	void* buffer;
	size_t bytes;
};

class Transmitter
{
public:
	static int sendMessage( int serviceId, Message msg );
};

class MsgObserver
{
	virtual void msgReceived( Message msg ) = 0;
};

class Receiver
{
public:
	static void addMsgObserver( int serviceId, MsgObserver* observer );

	static void rmvMsgObserver( int serviceId, MsgObserver* observer );

	// start eternal message waiting loop.
	static void waitMsg();

	// this will send a msg to the msg-waiting-thread to stop waiting.
	static void stopWaitingMsg();
};

#define MESSAGE_H
#endif