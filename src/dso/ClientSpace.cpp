/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "ClientSpace_Base.h"

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

#include <reef/rpc/INode.h>

#include <dso/IServerSpace.h>
#include <dso/ChangeSet.h>
#include <dso/NewObject.h>
#include <dso/IRemoteSpaceObserver.h>

#include <fstream>
#include <sstream>

namespace dso {

class ClientSpace : public ClientSpace_Base
{
public:
	ClientSpace()
	{
		_archiveObj = co::newInstance( "ca.LuaArchive" );

		_archiveObj->getService<ca::INamed>()->setName( "tmp.lua" );

		_archive = _archiveObj->getService<ca::IArchive>();

	}

	virtual ~ClientSpace()
	{
	}
	
	ca::ISpace* getSpace()
	{
		return _space.get();
	}
	
	void initialize( const std::string& _serverAddress, const std::string& _serverSpaceKey )
	{
		if( !_universe.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "universe not set" );
		}

		if( !_node.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "client node not set" );
		}

		if( _serverSpaceKey.empty() || _serverAddress.empty() )
		{
			CORAL_THROW( co::IllegalArgumentException, "Server information not set properly" );
		}

		co::RefPtr<co::IObject> object = _node->findRemoteInstance( "dso.ServerSpace", _serverSpaceKey, _serverAddress );

		
		if( !object )
		{
			CORAL_THROW( co::IllegalStateException, "Could not replicate space. space with key " << _serverSpaceKey << " not found on server " << _serverAddress  );
		}

		co::RefPtr<dso::IServerSpace> serverSpace = object->getService<dso::IServerSpace>();

		co::Range<co::int8 const> byteData = serverSpace->getPublishedSpaceData();

		std::ofstream of ( "tmp.lua" );

		for( int i = 0; i < byteData.getSize(); i++ )
		{
			of << byteData[i];
		}
		of.close();
		_archiveObj->setService( "model", _universe->getModel() );

		co::RefPtr<co::IObject> root = _archive->restore();

		co::RefPtr<co::IObject> spaceObj = co::newInstance( "ca.Space" );
		spaceObj->setService( "universe", _universe.get() );
		

		_space = spaceObj->getService<ca::ISpace>();
		_archive->getProvider()->setService( "model", _space->getModel() );
		
		_space->initialize( root.get() );
		initializeIds( _serverAddress, _serverSpaceKey );

		serverSpace->addRemoteSpaceObserver( this );
		
	}

	bool onRemoteSpaceChanged(  co::Range<const dso::ChangeSet> changes )
	{
		try
		{
			const std::string& script = "dso.SpaceSyncClient";
			const std::string& function = "applyReceivedChangeSet";
		
			co::Range<const co::Any> results;

			co::Any args[2];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].setArray(co::Any::AK_Range, co::typeOf<dso::ChangeSet>::get(), 0, ( (void*)changes.getStart() ), changes.getSize() );

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

	bool onNewObjects( co::Range<const dso::NewObject> newObjects )
	{
		try
		{
			const std::string& script = "dso.SpaceSyncClient";
			const std::string& function = "applyReceivedNewObjects";

			co::Range<const co::Any> results;

			co::Any args[4];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].setArray(co::Any::AK_Range, co::typeOf<dso::NewObject>::get(), 0, ( (void*)newObjects.getStart() ), newObjects.getSize() );

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

protected:

	ca::IUniverse* getUniverseService()
	{
		return _universe.get();
	}

	void setUniverseService( ca::IUniverse* universe )
	{
		_universe = universe;
	}

	reef::rpc::INode* getClientNodeService()
	{
		return _node.get();
	}
	
	void setClientNodeService( reef::rpc::INode* node )
	{
		_node = node;
	}
private:
	void initializeIds( const std::string& _serverAddress, const std::string& _serverSpaceKey )
	{
		const std::string& script = "dso.SpaceSyncClient";
		const std::string& function = "initializeIds";

		co::Range<const co::Any> results;

		co::IObject* object = _node->findRemoteInstance( "dso.ServerSpace", _serverSpaceKey, _serverAddress );

		dso::IServerSpace* serverSpace = object->getService<dso::IServerSpace>();

		co::Any args[1];
		args[0].set<ca::ISpace*>( _space.get() );

		co::getService<lua::IState>()->callFunction( script, function,
			co::Range<const co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
	}

private:
	co::RefPtr<ca::ISpace> _space;

	co::RefPtr<ca::IUniverse> _universe;
	co::RefPtr<reef::rpc::INode> _node;
	
	co::RefPtr<ca::IArchive> _archive;
	co::RefPtr<co::IObject> _archiveObj;

};

CORAL_EXPORT_COMPONENT( ClientSpace, ClientSpace );

} // namespace dso
