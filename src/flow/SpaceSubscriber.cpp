/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "SpaceSubscriber_Base.h"

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/Any.h>
#include <co/Log.h>
#include <co/IllegalStateException.h>
#include <co/IllegalArgumentException.h>
#include <lua/IState.h>

#include <ca/INamed.h>
#include <ca/IArchive.h>
#include <ca/IUniverse.h>
#include <ca/ISpace.h>
#include <ca/IOException.h>
#include <ca/IModel.h>
#include <ca/IGraphObserver.h>
#include <ca/IGraphChanges.h>

#include <flow/ISpacePublisher.h>
#include <flow/ChangeSet.h>
#include <flow/NewObject.h>

#include <fstream>
#include <sstream>

namespace flow {

class SpaceSubscriber : public SpaceSubscriber_Base
{
public:
	SpaceSubscriber()
	{
		ready = false;
	}

	virtual ~SpaceSubscriber()
	{
		_rootObject = 0;
		removeFromCache();
		_space = 0;
	}
	
	co::IObject* getRootObject()
	{
		return _rootObject.get();
	}
	

	void setSpace( ca::ISpace* space )
	{
		if( !space )
		{
			CORAL_THROW( co::IllegalArgumentException, "NULL space" );
		}
		removeFromCache();
		_space = space;
		initializeIds();

	}

	void onSubscribed(  co::Slice<co::int8> bytes, const std::string& modelName  )
	{
		ca::IModelRef model = getModel( modelName );

		if( model->getName() != modelName )
		{
			CORAL_THROW( co::IllegalStateException, "Space's model different from the publisher" );
		}

		std::ofstream of ( "tmp.lua" );

		for( int i = 0; i < bytes.getSize(); i++ )
		{
			of << bytes[i];
		}
		
		of.close();

		co::IObjectRef archiveObj = co::newInstance( "ca.LuaArchive" );
		archiveObj->getService<ca::INamed>()->setName( "tmp.lua" );

		archiveObj->setService( "model", model.get() );

		_rootObject = archiveObj->getService<ca::IArchive>()->restore();

		initializeIds();
	}
	
	void onPublish( co::Slice<flow::NewObject> newObjects, co::Slice<flow::ChangeSet> changes )
	{
		if( !ready )
		{
			CORAL_THROW( co::IllegalStateException, "Subscriber not ready" );
		}

		try
		{
			const std::string& script = "flow.SpaceSyncClient";
			const std::string& function = "applyReceivedChanges";
		
			co::Slice<co::Any> results;

			co::Any args[3];
			args[0] = _space.get();
			args[1] = newObjects;
			args[2] = changes;

			co::getService<lua::IState>()->call( script, function,
				args,
				results );
		}
		catch( std::exception& e )
		{
			CORAL_LOG(ERROR) << e.what();
			throw;
		}
	}

private:
	
	void initializeIds()
	{
		if( _space.isValid() && _space->getRootObject() == NULL && _rootObject.isValid() )
		{
			_space->initialize( _rootObject.get() );
		}

		if( _space.isValid() && _rootObject.isValid() && _space->getRootObject() == _rootObject.get() )
		{
			const std::string& script = "flow.SpaceSyncClient";
			const std::string& function = "initializeIds";

			co::Slice<co::Any> results;

			co::Any args[1];
			args[0] = _space.get();

			co::getService<lua::IState>()->call( script, function,
				args,
				results );
			ready = true;
		}
	}

	void removeFromCache()
	{
		if( _space.isValid() )
		{
			const std::string& script = "flow.SpaceSyncClient";
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


	ca::IModelRef getModel( const std::string& modelName )
	{
		if( _space.isValid() )
		{
			return _space->getUniverse()->getModel();
		}
		else
		{
			co::IObject* object = co::newInstance( "ca.Model" );
			ca::IModelRef model = object->getService<ca::IModel>();
			model->setName( modelName );
			return model;
		}
	}

private:
	co::IObjectRef _rootObject;
	ca::ISpaceRef _space;

	bool ready;

};

CORAL_EXPORT_COMPONENT( SpaceSubscriber, SpaceSubscriber );

} // namespace flow
