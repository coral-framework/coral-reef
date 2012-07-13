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

	bool onChange( co::uint32 serviceId, const std::string& memberName, const co::Any& newValue )
	{
		try
		{
			const std::string& script = "dso.SpaceSyncClient";
			const std::string& function = "applyReceivedChange";
		
			co::Range<const co::Any> results;

			co::Any args[4];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].set<co::uint32>( serviceId );
			args[2].set<const std::string&>( memberName );
			args[3].set<const co::Any&>( newValue );

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
	
	bool onNewObject( co::uint32 objectId, const std::string& typeName )
	{
		try
		{
			const std::string& script = "dso.SpaceSyncClient";
			const std::string& function = "applyReceivedNewObject";

			co::Range<const co::Any> results;

			co::Any args[4];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].set<co::uint32>( objectId );
			args[2].set<const std::string&>( typeName );

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

	bool onRefChange( co::uint32 serviceId, const std::string& memberName, const std::string& newRefValue )
	{
		try
		{
			const std::string& script = "dso.SpaceSyncClient";
			const std::string& function = "applyReceivedRefChange";

			co::Range<const co::Any> results;

			co::Any args[4];
			args[0].set<ca::ISpace*>( _space.get() );
			args[1].set<co::uint32>( serviceId );
			args[2].set<const std::string&>( memberName );
			args[3].set<const std::string&>( newRefValue );

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

		std::vector<const co::Any> results;

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
