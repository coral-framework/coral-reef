/*
 * Component implementation template for 'flow.ServiceIdMap'.
 * WARNING: remember to copy this file to your project dir before you begin to change it.
 * Generated by the Coral Compiler v0.8.0 on 2015-03-04 14:43:14.p
 */

#include "ServiceIdMap_Base.h"
#include <co/IObject.h>
#include <co/IComponent.h>
#include <co/IField.h>
#include <co/IReflector.h>
#include <co/IType.h>
#include <co/IArray.h>
#include <co/Log.h>

#include <ca/ISpace.h>
#include <ca/IModel.h>
#include <ca/IUniverse.h>

#include <algorithm>
#include <map>
#include <functional>
#include <set>

namespace flow {

class ServiceIdMap : public ServiceIdMap_Base
{
public:
	ServiceIdMap() : _currentId( 0 )
	{
		// empty constructor
	}

	virtual ~ServiceIdMap()
	{
		// empty destructor
	}

	//------ flow.IServiceIdMap Methods ------//

	co::int32 getId( co::IService* service )
	{
		auto it = _objectIdMap.find( service );
		if( it == _objectIdMap.end() )
			return 0;

		return it->second;
	}

	void getOrderedIds( co::IObject* root, std::vector<co::int32>& ids )
	{
		ids.clear();

		std::set<co::IService*> marked;

		genericGraphWalk( root, 
			[&]( co::IService* service ) -> bool 
			{
				if( !hasId( service ) )
				{
					if(co::isA<co::IObject>( service ) )
						objectId( service );
					else
						objectId( service->getProvider() );
				}

				if( marked.find( service ) == marked.end() )
				{
					ids.push_back( getId( service ) );
					marked.insert( service );
				}
			
				return false;
			}
			
		);
	}

	co::IService* getService( co::int32 id )
	{
		auto it = _idObjectMap.find( id );

		if( it == _idObjectMap.end() )
			return nullptr;
		
		return it->second.get();
	}

	bool hasId( co::IService* service )
	{
		return _objectIdMap.find( service ) != _objectIdMap.end();
	}

	void init( ca::ISpace* space )
	{
		assert( space && space->getUniverse());
		_model = space->getUniverse()->getModel();

		objectId( space->getRootObject() );
	}

	void initWithIds( ca::ISpace* space, co::Slice<co::int32> ids )
	{
		assert( space && space->getUniverse());
		_model = space->getUniverse()->getModel();

		genericGraphWalk( space->getRootObject(), 
			[&]( co::IService* service ) -> bool 
			{
				if( !hasId( service ) )
				{
					co::int32 id = ids.getFirst();
					insertInMap( service, id );
					ids.popFirst();
				}
				return false;
			}
		);

	}

	void insertInMap( co::IService* service, co::int32 id )
	{
		_idObjectMap.insert( IdObjectMap::value_type( id, service ) );
		_objectIdMap.insert( ObjectIdMap::value_type( service, id ) );
	}

	void objectId( co::IService* object )
	{
		genericGraphWalk( object, 
			[&]( co::IService* service ) -> bool 
			{
				if( !hasId( service ) )
				{
					insertInMap( service );
				}
				return false;
			}
		);
	}

	void removeObject( co::IObject* toRemove )
	{
		if( hasId( toRemove ) )
		{
			forEachPort( toRemove, [this, toRemove] ( co::IPortRef port )
			{
				co::IService* service = toRemove->getServiceAt( port.get() );
				
				if( service )
					_objectIdMap.erase( service );
			}
			);

			_objectIdMap.erase( toRemove );
		}
	}

	void shallowGivenObjectId( co::IObject* object, co::int32 id )
	{
		insertInMap( object, id );
		forEachPort( object, [this,object,&id]( co::IPortRef port )
			{
				if( port->getIsFacet() )
				{
					insertInMap( object->getServiceAt( port.get() ), ++id );
				}
			}
		);
	}

	void shallowObjectId( co::IObject* object )
	{
		insertInMap( object );
		
		forEachPort( object, [this, object]( co::IPortRef port )
			{
				if( port->getIsFacet() )
				{
					insertInMap( object->getServiceAt( port.get() ) );
				}
			}
		);
	}


private:

	void genericGraphWalk( co::IService* service, std::function<bool(co::IService*)> callback )
	{
		if( !service )
			return;

		bool stop = false;
		stop = callback( service );

		if( stop )
			return;

		if( co::isA<co::IObject>(service) )
		{
			co::IObject* object = co::cast<co::IObject>( service );

			forEachPort( object, [&]( co::IPortRef port )
				{

					if( port->getIsFacet() )
					{
						genericGraphWalk( object->getServiceAt( port.get() ), callback );
					}
					else
					{
						co::IService* receptacleService = object->getServiceAt( port.get() );
						
						if( receptacleService )
							genericGraphWalk( receptacleService->getProvider(), callback );
					}
				}
			);
		}
		else if( co::isA<co::IService>( service ) )
		{
			std::vector<co::IFieldRef> fields;
			_model->getFields( service->getInterface(), fields );

			std::for_each( fields.begin(), fields.end(), [=]( co::IFieldRef field )
				{
					co::TypeKind kind = field->getType()->getKind();
					if( kind == co::TK_INTERFACE )
					{
						co::IServiceRef serviceRef;
						getRefField( service, field.get(), serviceRef );
						if( serviceRef.isValid() )
							genericGraphWalk( serviceRef.get()->getProvider(), callback );
					}
					else if( kind == co::TK_ARRAY  )
					{
						co::IArray* arrayType = co::cast<co::IArray>( field->getType() );

						if( arrayType->getElementType()->getKind() == co::TK_INTERFACE )
						{
							co::AnyValue value;
							value.create( field->getType() );
							getRefVecField( service, field.get(), value.getAny() );
							for( int i = 0; i < value.getAny().getCount(); i++ )
							{
								genericGraphWalk( value.getAny()[i].get<co::IService*>()->getProvider(), callback );
							}
						}
					}
				}
			);
		}
	}

	void insertInMap( co::IService* service )
	{
		insertInMap( service, ++_currentId );
	}

	void getRefField( co::IService* service, co::IField* field, co::IServiceRef& ref )
	{
		service->getInterface()->getReflector()->getField( service, field, ref );
	}

	void getRefVecField( co::IService* service, co::IField* field, const co::Any& refs )
	{
		service->getInterface()->getReflector()->getField( service, field, refs );
	}

	void forEachPort( co::IObject* object, std::function<void(co::IPortRef)> callback )
	{
		std::vector<co::IPortRef> ports;
		_model->getPorts( object->getComponent(), ports );

		std::for_each( ports.begin(), ports.end(), callback );
	}

private:
	typedef std::map<co::int32, co::IServiceRef> IdObjectMap;
	typedef std::map<co::IServiceRef, co::int32> ObjectIdMap;

private:
	co::uint32 _currentId;
	ca::IModelRef _model;
	IdObjectMap _idObjectMap;
	ObjectIdMap _objectIdMap;
};

CORAL_EXPORT_COMPONENT( ServiceIdMap, ServiceIdMap );

} // namespace flow
