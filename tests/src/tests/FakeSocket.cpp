#include "FakeSocket.h"
#include <co/Exception.h>

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
        MsgTarget* _target;

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

		void setTarget( MsgTarget* target )
		{
			_target = target;
		}

		void notifyTarget()
		{
			if( !_target )
				throw co::Exception( "A message sent needs a Target ServerNode" );
            
            _target->msgSent();
		}
	};

	/*----------------------------------------------------------
	----------------- FakeSocket impl --------------------------
	----------------------------------------------------------*/
	static std::map<std::string, BinderQueue*> _bound;

    void FakeSocket::setMsgTarget( MsgTarget* target, const std::string& address )
	{
		BinderQueue* bq = getQueueAt( address );
		bq->setTarget( target );
	}

	void FakeSocket::sendAt( const std::string& msg, const std::string& binderAddress )
	{
		BinderQueue* bq = getQueueAt( binderAddress );
		bq->insertMsg( msg );
        bq->notifyTarget();
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