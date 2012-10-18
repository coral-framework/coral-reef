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
		subscriber->onSubscribed( getPublishedSpaceData(), _space->getUniverse()->getModel()->getName() );
		_subscribers.push_back( subscriber );
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

		co::Range<co::Any> results;

		co::Any args[3];
		args[0] = _space.get();
		args[1] = co::Any( _allChanges ).asIn();
		args[2] = co::Any( _subscribers ).asIn();
		
		co::getService<lua::IState>()->call( script, function,
			co::Range<co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
		
		_allChanges.clear();

	}
	
protected:

	void onGraphChanged( ca::IGraphChanges* changes )
	{
		_allChanges.push_back( changes );
	}

private:

	co::Range<co::int8> getPublishedSpaceData()
	{
		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "No space published" );
		}
		_archiveObj->setService( "model", _space->getModel() );
		_archive->save( _space->getRootObject() );

		std::ifstream ifs( "serverTmp.lua" );

		data.assign((std::istreambuf_iterator<char>(ifs)),
                 std::istreambuf_iterator<char>());
		ifs.close();
		return data;
	}

	void initializeIds()
	{
		const std::string& script = "flow.SpaceSyncServer";
		const std::string& function = "initializeIds";

		co::Range<co::Any> results;

		co::Any args[1];
		args[0] = _space.get();

		co::getService<lua::IState>()->call( script, function,
			co::Range<co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
	}
private:
	co::RefPtr<ca::ISpace> _space;
	co::RefPtr<ca::IArchive> _archive;
	co::RefPtr<co::IObject> _archiveObj;
	co::RefVector<ca::IGraphChanges> _allChanges;
	
	co::RefVector<flow::ISpaceSubscriber> _subscribers;
	std::vector<co::int8> data;
};

CORAL_EXPORT_COMPONENT( SpacePublisher, SpacePublisher );

} // namespace flow
