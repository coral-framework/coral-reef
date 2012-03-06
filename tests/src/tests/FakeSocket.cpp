#include "FakeSocket.h"
#include <co/Exception.h>

#include <Message.pb.h>

#include <map>
#include <queue>

namespace reef
{
	struct BinderQueue
	{
		std::string reply;
		bool hasReply;
		std::queue<std::string> msgs;
		int numWatchers;
		const ReplyDelegate* repDel;

		BinderQueue() : hasReply( false ), numWatchers( 1 )
		{
		}

		void popMsg( std::string& msg )
		{
			msg = msgs.front();
			msgs.pop();
		}

		void insertMsg( const std::string& msg )
		{
			msgs.push( msg );
		}

		void postReply( const std::string& msg )
		{
			hasReply = true;
			reply = msg;
		}

		bool receiveReply( std::string& msg )
		{
			if( hasReply )
			{
				msg = reply;
				hasReply = false;
				return true;
			}
			return false;
		}

		void setRepDel( const ReplyDelegate* replyDelegate )
		{
			if( !repDel )
				throw co::Exception( "only one reply delegate must be set at a time" );

			repDel = replyDelegate;
		}

		void messageRequiresReply()
		{
			if( !repDel )
				throw co::Exception( "A message that requires reply needs a reply delegate" );

			repDel->replyableMsgSent();
			repDel = 0;
		}
	};

	/*----------------------------------------------------------
	----------------- FakeSocket impl --------------------------
	----------------------------------------------------------*/
	static std::map<std::string, BinderQueue*> _bound;

    void FakeSocket::setReplyDelegateAt( const ReplyDelegate* repDel, const std::string& address )
	{
		BinderQueue* bq = getQueueAt( address );
		bq->setRepDel( repDel );
	}

	void FakeSocket::sendAt( const std::string& msg, const std::string& binderAddress )
	{
		BinderQueue* bq = getQueueAt( binderAddress );
		bq->insertMsg( msg );

		// if the message requires a reply, the replyDelegate must be warned
		Message message;
		message.ParseFromString( msg );
		Message::Type type = message.type();

		if( type == Message::TYPE_NEW )
			bq->messageRequiresReply();
		else
		{
			const Message_Member& callMsg = message.msgmember();
			if( callMsg.hasreturn() )
				bq->messageRequiresReply();
		}
	}

	void FakeSocket::receiveAt( std::string& msg, const std::string& binderAddress )
	{
		BinderQueue* bq = getQueueAt( binderAddress );
		bq->popMsg( msg );
	}

	void FakeSocket::reply( const std::string& msg, const std::string& binderAddress )
	{
		BinderQueue* bq = getQueueAt( binderAddress );
		bq->postReply( msg );
	}

	void FakeSocket::receiveReply( std::string& msg, const std::string& binderAddress )
	{
		BinderQueue* bq = getQueueAt( binderAddress );
		if( !bq->receiveReply( msg ) )
			throw co::Exception( 
			"No reply to receive. In the testing environment the send/reply must be called in perfect order"
			);
	}

    void FakeSocket::bindOrConnectAt( const std::string& address )
    {
		std::map<std::string, BinderQueue*>::iterator it = _bound.find( address );
        if( it == _bound.end() )
			_bound.insert( std::pair<std::string,BinderQueue*>( address, new BinderQueue() ) );
		else
			(*it).second->numWatchers = (*it).second->numWatchers + 1;
    }

	void FakeSocket::closeAt( const std::string& address )
	{
		std::map<std::string, BinderQueue*>::iterator it = _bound.find( address );
        if( it == _bound.end() )
			throw co::Exception( "address not bound/connected when trying to close" );
        
        BinderQueue* bq = (*it).second;
		if( bq->numWatchers <= 1 )
		{
			delete bq;
			_bound.erase( it );
		}
		else
		{
			bq->numWatchers = bq->numWatchers - 1;
		}
	}

	BinderQueue* FakeSocket::getQueueAt( const std::string& address )
	{
		std::map<std::string, BinderQueue*>::iterator it = _bound.find( address );
        if( it == _bound.end() )
			throw co::Exception( "address not bound/connected" );
        
        return (*it).second;
	}
}