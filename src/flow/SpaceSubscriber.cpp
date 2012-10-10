/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "SpaceSubscriber_Base.h"

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/RefPtr.h>
#include <co/Range.h>
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
		_archiveObj = co::newInstance( "ca.LuaArchive" );

		_archiveObj->getService<ca::INamed>()->setName( "tmp.lua" );

		_archive = _archiveObj->getService<ca::IArchive>();

		ready = false;

	}

	virtual ~SpaceSubscriber()
	{
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
		_space = space;
		initializeIds();

	}

	bool onSubscribed( co::Range<co::int8 const> byteData, const std::string& modelName  )
	{
		ca::IModel* model = getModel( modelName ).get();

		if( model->getName() != modelName )
		{
			CORAL_THROW( co::IllegalStateException, "Space's model different from the publisher" );
		}

		std::ofstream of ( "tmp.lua" );

		for( int i = 0; i < byteData.getSize(); i++ )
		{
			of << byteData[i];
		}
		
		of.close();

		_archiveObj->setService( "model", model );

		_rootObject = _archive->restore();

		initializeIds();

		return true;
	}
	
	bool onPublish( co::Range<const flow::NewObject> newObjects, co::Range<const flow::ChangeSet> changes )
	{
		if( !ready )
		{
			return false;	
		}

		try
		{
			const std::string& script = "flow.SpaceSyncClient";
			const std::string& function = "applyReceivedChanges";
		
			co::Range<const co::Any> results;

			co::Any args[3];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].setArray(co::Any::AK_Range, co::typeOf<flow::NewObject>::get(), 0, ( (void*)newObjects.getStart() ), newObjects.getSize() );
			args[2].setArray(co::Any::AK_Range, co::typeOf<flow::ChangeSet>::get(), 0, ( (void*)changes.getStart() ), changes.getSize() );

			co::getService<lua::IState>()->callFunction( script, function,
				co::Range<const co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
				results );
		}
		catch( std::exception& e )
		{
			CORAL_LOG(ERROR) << e.what();
			return false;
		}
		return true;
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

			co::Range<const co::Any> results;

			co::Any args[1];
			args[0].set<ca::ISpace*>( _space.get() );

			co::getService<lua::IState>()->callFunction( script, function,
				co::Range<const co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
				results );
			ready = true;
		}
	}

	co::RefPtr<ca::IModel> getModel( const std::string& modelName )
	{
		if( _space.isValid() )
		{
			return _space->getUniverse()->getModel();
		}
		else
		{
			co::IObject* object = co::newInstance( "ca.Model" );
			_model = object->getService<ca::IModel>();
			_model->setName( modelName );
			return _model;
		}

	}

private:
	co::RefPtr<co::IObject> _rootObject;
	co::RefPtr<ca::ISpace> _space;
	
	co::RefPtr<ca::IArchive> _archive;
	co::RefPtr<co::IObject> _archiveObj;

	co::RefPtr<ca::IModel> _model;

	bool ready;

};

CORAL_EXPORT_COMPONENT( SpaceSubscriber, SpaceSubscriber );

} // namespace flow
