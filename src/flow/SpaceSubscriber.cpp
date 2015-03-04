/*
 * Calcium - Domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "SpaceSubscriber_Base.h"

#include <co/Coral.h>
#include <co/IObject.h>
#include <co/Any.h>
#include <co/Log.h>
#include <co/IMember.h>
#include <co/IField.h>
#include <co/IReflector.h>
#include <co/IllegalStateException.h>
#include <co/IllegalArgumentException.h>

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
#include <flow/IServiceIdMap.h>

#include <fstream>
#include <sstream>
#include <ctime>
#include <set>
#include <algorithm>

namespace flow {

class SpaceSubscriber : public SpaceSubscriber_Base
{
public:
	SpaceSubscriber()
	{
		ready = false;
		_ids = "";
	}

	virtual ~SpaceSubscriber()
	{
		_rootObject = 0;
		
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
		
		_space = space;
		initializeIds();
	}

	void onSubscribed(  const std::string& bytes,  const std::string& ids, const std::string& modelName  )
	{
		ca::IModelRef model = getModel( modelName );

		if( model->getName() != modelName )
		{
			CORAL_THROW( co::IllegalStateException, "Space's model different from the publisher" );
		}

		std::ofstream of ( "tmp.lua" );

		of << bytes;
		
		of.close();

		co::IObjectRef archiveObj = co::newInstance( "ca.LuaArchive" );
		archiveObj->getService<ca::INamed>()->setName( "tmp.lua" );

		archiveObj->setService( "model", model.get() );

		_rootObject = archiveObj->getService<ca::IArchive>()->restore();
		_ids = ids;
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
			applyReceivedChanges( newObjects, changes );
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
			_graphIds = co::newInstance( "flow.ServiceIdMap" )->getService<flow::IServiceIdMap>();
			_graphIds->initWithIds( _space.get(), getNumbers(_ids.substr(1, _ids.npos-1) ) );
			ready = true;
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
	void applyReceivedChanges( co::Slice<flow::NewObject> newObjects, co::Slice<flow::ChangeSet> changes )
	{
		std::set<co::IService*> newObjsCoral;
		applyReceivedNewObjects( newObjects, newObjsCoral );
		applyReceivedChangeSet( changes, newObjsCoral );
	}

	void applyReceivedChangeSet( co::Slice<flow::ChangeSet> changes, const std::set<co::IService*>& newObjsCoral )
	{

		std::for_each( changes.begin(), changes.end(), [=] (const ChangeSet& changeSet)
		
			{
				co::IService* service = _graphIds->getService( changeSet.serviceId );
				if( service )
				{
					const std::vector<flow::Change>& changeList = changeSet.changes;
					for( int i = 0; i < changeList.size(); i++ )
					{
						const Change& currentChange = changeList[i];
						if( currentChange.newRefValue == "" )
						{
							setField( service, currentChange.name, currentChange.newValue );
						}
						else
						{
							applyRefChange( service, currentChange.name, currentChange.newRefValue );
						}
					}
				}
			} );

		/*for( flow::ChangeSet changeSet = changes.getFirst();changes; changes.popFirst() )
		{
			co::IService* service = _graphIds->getService( changeSet.serviceId );
			if( service )
			{
				const std::vector<flow::Change>& changeList = changeSet.changes;
				for( int i = 0; i < changeList.size(); i++ )
				{
					const Change& currentChange = changeList[i];
					if( currentChange.newRefValue == "" )
					{
						setField( service, currentChange.name, currentChange.newValue );
					}
					else
					{
						applyRefChange( space, service, currentChange.name, currentChange.newRefValue );
					}
				}
			}
		}*/
		
		for( ;changes; changes.popFirst() )
		{
			flow::ChangeSet changeSet = changes.getFirst();
			co::IService* service = _graphIds->getService( changeSet.serviceId );

			if( newObjsCoral.find( service->getProvider() ) == newObjsCoral.end() )
			{
				_space->addChange( service );
			}

		}

		_space->notifyChanges();
	}

	void applyRefChange( co::IService* service, const std::string& name, const std::string& idList )
	{
		if( idList == "nil" )
		{
			co::IService* serviceNil = nullptr;
			co::AnyValue value(serviceNil); // null
			setField( service, name, value );
		}

		if( idList.at( 0 ) == '#' )
		{
			if( idList.at( 1 ) == '{' )
			{
				std::string numbersStr = idList.substr( 2, idList.npos - 2 );

				std::vector<int> numbers = getNumbers( numbersStr );

				co::IField* refVecField = co::cast<co::IField>( service->getInterface()->getMember( name ) );
				co::IType* fieldType = refVecField->getType();

				co::AnyValue returnValue;
				returnValue.create( fieldType );

				co::IReflector* serviceReflector = service->getInterface()->getReflector();

				serviceReflector->getField( service, refVecField, returnValue );

				returnValue.getAny().resize( numbers.size() );
				for( int i = 0; i < numbers.size(); i++ )
				{
					co::IService* serviceRef = _graphIds->getService( numbers[i] );
					if( !serviceRef )
						CORAL_THROW( co::Exception, "service id " << numbers[i] << " not found" );
					returnValue.getAny()[i].put( serviceRef );
				}

				refVecField->getOwner()->getReflector()->setField( service, refVecField, returnValue );
			}
			else
			{
				int id = atoi( idList.substr( 1 ).c_str() );
				co::IService* serviceValue = _graphIds->getService( id );
				setField( service, name, serviceValue );
			}
		}

	}

	void applyReceivedNewObjects( co::Slice<flow::NewObject> newObjects, std::set<co::IService*>& newObjsCoral )
	{
		for(;newObjects; newObjects.popFirst() )
		{
			flow::NewObject newObj = newObjects.getFirst();
			co::IObject* newObjCoral = co::newInstance( newObj.typeName );
			newObjsCoral.insert( newObjCoral );

			_graphIds->shallowGivenObjectId( newObjCoral, newObj.newId );

			assert( _graphIds->getId( newObjCoral ) == newObj.newId );

		}
	}

	void setField( co::IService* service, const std::string& name, const co::Any& value )
	{
		if( co::isA<co::IObject>( service ) )
		{
			co::IObject* object = co::cast<co::IObject>( service );
			object->setService( name, value.get<co::IService*>() );
		}
		else
		{
			co::IInterface* serviceInterface = service->getInterface();
			co::IField* field = co::cast<co::IField>( serviceInterface->getMember( name ) );
			field->getOwner()->getReflector()->setField( service, field, value );
		}
	}

	std::vector<int> getNumbers( const std::string& str )
	{
		std::vector<int> numbers;

		std::istringstream ss(str);
		std::string token;

		while(std::getline(ss, token, ',')) {
			numbers.push_back( atoi( token.c_str() ) );
		}

		numbers.pop_back();

		return numbers;
	}

private:
	co::IObjectRef _rootObject;
	ca::ISpaceRef _space;

	flow::IServiceIdMapRef _graphIds;
	std::string _ids;

	bool ready;

};

CORAL_EXPORT_COMPONENT( SpaceSubscriber, SpaceSubscriber );

} // namespace flow
