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
#include <co/IReflector.h>
#include <co/IField.h>

#include <ca/IUniverse.h>
#include <ca/IOException.h>
#include <ca/ISpace.h>
#include <ca/IModel.h>
#include <ca/IArchive.h>
#include <ca/INamed.h>
#include <ca/IGraphChanges.h>
#include <ca/IObjectChanges.h>
#include <ca/IServiceChanges.h>
#include <ca/ChangedConnection.h>
#include <ca/ChangedRefField.h>
#include <ca/ChangedRefVecField.h>
#include <ca/ChangedValueField.h>

#include <flow/ISpaceSubscriber.h>
#include <flow/ChangeSet.h>
#include <flow/NewObject.h>
#include <flow/IServiceIdMap.h>

#include <fstream>
#include <sstream>
#include <ctime>

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
		
		_space = space;

		_graphIds = co::newInstance( "flow.ServiceIdMap" )->getService<flow::IServiceIdMap>();

		_space->addGraphObserver( this );
		_graphIds->init( _space.get() );

	}
	
	void addSubscriber( flow::ISpaceSubscriber* subscriber )
	{
		if( !_space.isValid() )
		{
			CORAL_THROW( co::IllegalStateException, "NULL space" );
		}

		calculateData();
		
		getOrderedIds();
		subscriber->onSubscribed( _data, serializeStringIds(), _space->getUniverse()->getModel()->getName() );
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
		
		processAllSpaceChanges();
		
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

	void getOrderedIds()
	{
		_graphIds->getOrderedIds( _space->getRootObject(), _ids );
	}

private:
	typedef std::map<int, std::vector<flow::Change>> IdChangeMap;

private:
	void processAllSpaceChanges()
	{
		
		if( _subscribers.empty() )
			return;

		std::set<co::IObject*> newObjs;

		IdChangeMap accumulatedChanges;

		for( int i = 0; i < _allChanges.size(); i++ )
		{
			processNewObjects( _allChanges[i].get(), newObjs );
		}

		std::vector<flow::NewObject> newObjects = generateValueForNewObjects( newObjs, accumulatedChanges );

		for( int i = 0; i < _allChanges.size(); i++ )
		{
			processChanges( _allChanges[i].get(), accumulatedChanges );
		}

		std::vector<flow::ChangeSet> changeSetArray;
		for( auto it = accumulatedChanges.begin(); it != accumulatedChanges.end(); it++ )
		{
			flow::ChangeSet changeSet;
			changeSet.serviceId = it->first;
			changeSet.changes = it->second;
			changeSetArray.push_back( changeSet );
		}

		std::sort( newObjects.begin(), newObjects.end(), [](const flow::NewObject& a, const flow::NewObject& b) 
					{ 
						return a.newId < b.newId;
					} 
		);

		std::sort( changeSetArray.begin(), changeSetArray.end(), [](const flow::ChangeSet& a, const flow::ChangeSet& b) 
					{ 
						return a.serviceId < b.serviceId;
					} 
		);

		for( auto observer = _subscribers.begin(); observer != _subscribers.end(); observer++ )
		{
			observer->get()->onPublish( newObjects, changeSetArray );
		}

	}

	void processChanges( ca::IGraphChanges* changes, IdChangeMap& resultChangesTable )
	{
		co::TSlice<ca::IObjectChanges*> objectChanges = changes->getChangedObjects();

		for(;objectChanges;objectChanges.popFirst() )
		{
			ca::IObjectChanges* current = objectChanges.getFirst();
			co::IObject* object = current->getObject();

			if( !_graphIds->hasId( object ) )
			{
				return;
			}

			int objectId = _graphIds->getId( object );

			for( auto connChanges = current->getChangedConnections(); connChanges; connChanges.popFirst() )
			{
				ca::ChangedConnection changedConn = connChanges.getFirst();
				auto& changeList = resultChangesTable[objectId];
				changeList.push_back( createChange( changedConn.receptacle.get(), changedConn.current ) );
			}

			for( auto connChanges = current->getChangedServices(); connChanges; connChanges.popFirst() )
			{
				ca::IServiceChanges* changedService = connChanges.getFirst();
				processServiceChanges( changedService, resultChangesTable );
			}


		}

	}

	void processServiceChanges( ca::IServiceChanges* serviceChanges, IdChangeMap& resultChangesTable )
	{
		int serviceId = _graphIds->getId( serviceChanges->getService() );
		
		auto& allChanges = resultChangesTable[serviceId];


		for( auto changedValues = serviceChanges->getChangedValueFields(); changedValues; changedValues.popFirst() )
		{
			auto changedValue = changedValues.getFirst();
			allChanges.push_back( createChange( changedValue.field.get(), changedValue.current ) );
		}

		for( auto changedValues = serviceChanges->getChangedRefFields(); changedValues; changedValues.popFirst() )
		{
			auto changedValue = changedValues.getFirst();
			allChanges.push_back( createChange( changedValue.field.get(), changedValue.current ) );
		}

		for( auto changedValues = serviceChanges->getChangedRefVecFields(); changedValues; changedValues.popFirst() )
		{
			auto changedValue = changedValues.getFirst();
			allChanges.push_back( createChange( changedValue.field.get(), changedValue.current ) );
		}
	}

	void processNewObjects( ca::IGraphChanges* changes, std::set<co::IObject*>& newObjectsSet )
	{
		co::TSlice<co::IObject*> addedObjects = changes->getAddedObjects();

		for( ;addedObjects; addedObjects.popFirst() ){
			co::IObject* current = addedObjects.getFirst();
			newObjectsSet.insert( current );
		}

		co::TSlice<co::IObject*> removedObjects = changes->getRemovedObjects();

		for( ;removedObjects; removedObjects.popFirst() ){
			co::IObject* current = removedObjects.getFirst();
			if( !newObjectsSet.erase( current ) )
			{
				_graphIds->removeObject( current );
			}
		}
	}

	std::vector<flow::NewObject> generateValueForNewObjects( const std::set<co::IObject*>& newObjects, IdChangeMap& resultChangesTable )
	{
		std::vector<flow::NewObject> result;
		for( auto it = newObjects.begin(); it != newObjects.end(); it++ )
		{
			co::IObject* newObject = *it;
			_graphIds->shallowObjectId( newObject );
			flow::NewObject no;
			no.newId = _graphIds->getId( newObject );
			no.typeName = newObject->getComponent()->getFullName();
			result.push_back( no );
		}

		for( auto it = newObjects.begin(); it != newObjects.end(); it++ )
		{
			getNewObjectChanges( *it, resultChangesTable );
		}
		return result;

	}

	void getNewObjectChanges( co::IObject* newObject, IdChangeMap& resultChangesTable )
	{
		int objectId = _graphIds->getId( newObject );

		std::vector<co::IPortRef> ports;
		_space->getModel()->getPorts( newObject->getComponent(), ports );

		for( auto it = ports.begin(); it != ports.end(); it++ )
		{
			if( it->get()->getIsFacet() )
			{
				getNewServiceChanges( newObject->getServiceAt( (*it).get() ), resultChangesTable );
			}
			else
			{
				co::IService* service = newObject->getServiceAt( it->get() );
				if( service )
				{
					std::vector<flow::Change>& receptacleChanges = resultChangesTable[objectId];
					receptacleChanges.push_back( createChange( it->get(), service ) );
				}
			}
		}

	}

	void getNewServiceChanges( co::IService* newService,  IdChangeMap& resultChangesTable )
	{
		int serviceId = _graphIds->getId( newService );
		std::vector<co::IFieldRef> fields;
		_space->getModel()->getFields( newService->getInterface(), fields );

		std::vector<flow::Change>& fieldChanges = resultChangesTable[serviceId];
		
		for( auto it = fields.begin(); it != fields.end(); it++ )
		{
			co::AnyValue fieldValue; 
			auto field = (*it).get();
			field->getOwner()->getReflector()->getField( newService, field, fieldValue );
			
			fieldChanges.push_back( createChange( field, fieldValue.getAny().asIn() ) );
		}

	}

	flow::Change createChange( co::IMember* member, const co::Any& value )
	{
		flow::Change change;
		change.name = member->getName();

		co::IType* memberType = nullptr;

		if( co::isA<co::IField>(member) )
		{
			memberType = co::cast<co::IField>(member)->getType();
		}
		else
		{
			memberType = co::cast<co::IPort>(member)->getType();
		}


		if( memberType->getKind() == co::TK_INTERFACE )
		{
			if( value.isNull() )
			{
				change.newRefValue = "nil";
			}
			else
			{
				co::IService* service = value.get<co::IService*>();
				if( service == nullptr )
				{
					change.newRefValue = "nil";
				}
				else
				{
					int id = _graphIds->getId( service );
					std::stringstream ss;
					ss << "#" << id;
					change.newRefValue = ss.str();
				}
			}
		}
		else if( memberType->getKind() == co::TK_ARRAY && co::cast<co::IArray>( memberType )->getElementType()->getKind() == co::TK_INTERFACE )
		{
			std::stringstream ss;
			ss << "#{";

			int size = value.getCount();

				for( int i = 0; i < size; i++ )
				{
					co::IService* service = value.at( i ).get<co::IService*>();
					co::uint32 id = _graphIds->getId( service );
					ss << id << ",";

				}

			ss << "}";
			change.newRefValue = ss.str();
		}
		else
		{
			change.newValue = value;
		}
		return change;
	}

	std::string serializeStringIds()
	{
		std::stringstream ss;
		ss << "{";

		for( int i = 0; i < _ids.size(); i++ )
		{
			ss << _ids[i] << ",";
		}

		ss << "}";
		return ss.str();
	}

private:
	ca::ISpaceRef _space;
	ca::IArchiveRef _archive;
	co::IObjectRef _archiveObj;
	std::vector<ca::IGraphChangesRef> _allChanges;

	flow::IServiceIdMapRef _graphIds;
	
	std::vector<flow::ISpaceSubscriberRef> _subscribers;
	std::string _data;
	std::vector<co::int32> _ids;
	std::vector<co::Any> _args;
};


CORAL_EXPORT_COMPONENT( SpacePublisher, SpacePublisher );

} // namespace flow
