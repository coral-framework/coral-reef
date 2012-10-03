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

#include <rpc/INode.h>

#include <flow/IServerSpace.h>
#include <flow/ChangeSet.h>
#include <flow/NewObject.h>
#include <flow/IRemoteSpaceObserver.h>

#include <fstream>
#include <sstream>

namespace flow {

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
	
	co::IObject* getRootObject()
	{
		return _rootObject.get();
	}
	

	void setSpace( ca::ISpace* space )
	{
		_space = space;
	}

	ca::ISpace* getSpace()
	{
		return _space.get();
	}
	
	bool initializeData( co::Range<co::int8 const> byteData, const std::string& modelName  )
	{
		std::ofstream of ( "tmp.lua" );

		for( int i = 0; i < byteData.getSize(); i++ )
		{
			of << byteData[i];
		}
		of.close();
		_archiveObj->setService( "model", getModel( modelName ).get() );

		_rootObject = _archive->restore();

		return true;
	}

	void registerRemoteSpaceObserver( const std::string& _serverAddress, const std::string& _serverSpaceKey )
	{
		if( !_node.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "client node not set" );
		}

		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "local space not set" );
		}

		if( _serverSpaceKey.empty() || _serverAddress.empty() )
		{
			CORAL_THROW( co::IllegalArgumentException, "Server information not set properly" );
		}

		co::RefPtr<co::IObject> object = _node->findRemoteInstance( "flow.ServerSpace", _serverSpaceKey, _serverAddress );
		
		if( !object )
		{
			CORAL_THROW( co::IllegalStateException, "Could not replicate space. space with key " << _serverSpaceKey << " not found on server " << _serverAddress  );
		}

		co::RefPtr<flow::IServerSpace> serverSpace = object->getService<flow::IServerSpace>();

		_space->initialize( getRootObject() );

		initializeIds();
		serverSpace->addRemoteSpaceObserver( this );

	}
	
	bool onRemoteSpaceChanged( co::Range<const flow::NewObject> newObjects, co::Range<const flow::ChangeSet> changes )
	{
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

protected:

	rpc::INode* getClientNodeService()
	{
		return _node.get();
	}
	
	void setClientNodeService( rpc::INode* node )
	{
		_node = node;
	}
private:
	void initializeIds()
	{
		const std::string& script = "flow.SpaceSyncClient";
		const std::string& function = "initializeIds";

		co::Range<const co::Any> results;

		co::Any args[1];
		args[0].set<ca::ISpace*>( _space.get() );

		co::getService<lua::IState>()->callFunction( script, function,
			co::Range<const co::Any>( args, CORAL_ARRAY_LENGTH( args ) ),
			results );
	}

	co::RefPtr<ca::IModel> getModel( const std::string& modelName )
	{
		co::IObject* modelObj = co::newInstance( "ca.Model" );
		co::RefPtr<ca::IModel> model = modelObj->getService<ca::IModel>();
		model->setName( modelName );
		return model;
	}

private:
	co::RefPtr<co::IObject> _rootObject;
	co::RefPtr<ca::ISpace> _space;

	co::RefPtr<rpc::INode> _node;
	
	co::RefPtr<ca::IArchive> _archive;
	co::RefPtr<co::IObject> _archiveObj;

};

CORAL_EXPORT_COMPONENT( ClientSpace, ClientSpace );

} // namespace flow
