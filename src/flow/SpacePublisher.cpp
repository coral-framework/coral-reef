/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "SpacePublisher_Base.h"

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IllegalArgumentException.h>
#include <co/IllegalStateException.h>
#include <co/Log.h>

#include <ca/IUniverse.h>
#include <ca/IOException.h>
#include <ca/ISpace.h>
#include <ca/IModel.h>
#include <ca/IArchive.h>
#include <ca/INamed.h>
#include <ca/IGraphChanges.h>

#include <flow/ISpaceSubscriber.h>
#include <flow/ChangeSet.h>

#include <lua/IState.h>

#include <fstream>
#include <sstream>

#include <map>
#include <set>
#include <algorithm>

namespace flow {

class SpacePublisher : public SpacePublisher_Base
{
public:
	SpacePublisher()
	{
		_archiveObj = co::newInstance( "ca.LuaArchive" );
		_archiveObj->getService<ca::INamed>()->setName( "serverTmp.lua" );

		_archive = _archiveObj->getService<ca::IArchive>();
	}

	virtual ~SpacePublisher()
	{
		if( _space.isValid() )
		{
			_space->removeGraphObserver( this );
		}
	}
	
	ca::ISpace* getSpace()
	{
		return _space.get();
	}
	
	void setSpace( ca::ISpace* space )
	{
		if( space == NULL )
		{
			CORAL_THROW( co::IllegalArgumentException, "NULL space" );
		}

		if( _space.isValid() )
		{
			_space->removeGraphObserver( this );
		}
		
		removeFromCache();
		_space = space;
		_space->addGraphObserver( this );
		initializeIds();
	}
	
	void addSubscriber( flow::ISpaceSubscriber* subscriber )
	{
		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "NULL space" );
		}
		calculateData();
		subscriber->onSubscribed( _data, _space->getUniverse()->getModel()->getName() );
		_subscribers.push_back( subscriber );
		_data.clear();
	}

	void removeSubscriber( flow::ISpaceSubscriber* subscriber )
	{
		std::vector<flow::ISpaceSubscriberRef>::iterator position = std::find( _subscribers.begin(), _subscribers.end(), subscriber );
		if( position != _subscribers.end() )
		{
			_subscribers.erase( position );
		}
	}

	void clearSubscribers()
	{
		_subscribers.clear();
	}

	void publish()
	{
		if( _subscribers.empty() )
		{
			CORAL_LOG(WARNING) << "No subscribers";
		}

		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "Space was not replicated from any server" );
		}

		if( _allChanges.empty() )
		{
			return;
		}

		const std::string& script = "flow.SpaceSyncServer";
		const std::string& function = "processAllSpaceChanges";

		co::Slice<co::Any> results;

		co::Any args[3];
		args[0] = _space.get();
		args[1] = co::Slice<ca::IGraphChanges*>( _allChanges );
		args[2] = co::Slice<flow::ISpaceSubscriber*>( _subscribers );
		
		co::getService<lua::IState>()->call( script, function,
			co::Slice<co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
		
		_allChanges.clear();
	}
	
protected:

	void onGraphChanged( ca::IGraphChanges* changes )
	{
		_allChanges.push_back( changes );
	}

private:

	void calculateData()
	{
		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "No space published" );
		}
		_archiveObj->setService( "model", _space->getModel() );
		_archive->save( _space->getRootObject() );

		std::ifstream ifs( "serverTmp.lua" );

		_data.assign((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
		ifs.close();
	}

	void initializeIds()
	{
		const std::string& script = "flow.SpaceSyncServer";
		const std::string& function = "initializeIds";

		co::Slice<co::Any> results;

		co::Any args[1];
		args[0] = _space.get();

		co::getService<lua::IState>()->call( script, function,
			co::Slice<co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
	}

	void removeFromCache()
	{
		if( _space.isValid() )
		{
			const std::string& script = "flow.SpaceSyncServer";
			const std::string& function = "removeFromCache";
		
			co::Slice<co::Any> results;

			co::Any args[1];
			args[0] = _space;

			co::getService<lua::IState>()->call( script, function,
				args,
				results );
			co::getService<lua::IState>()->collectGarbage();
		}
	}

private:
	ca::ISpaceRef _space;
	ca::IArchiveRef _archive;
	co::IObjectRef _archiveObj;
	std::vector<ca::IGraphChangesRef> _allChanges;
	
	std::vector<flow::ISpaceSubscriberRef> _subscribers;
	std::string _data;
};

CORAL_EXPORT_COMPONENT( SpacePublisher, SpacePublisher );

} // namespace flow
