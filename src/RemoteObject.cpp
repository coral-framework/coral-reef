#include "RemoteObject.h"

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>

static void printServiceInfo( co::IService* service, const char* msg )
{
    printf( "Service name: %s %s\n", service->getInterface()->getFullName().c_str(), msg );
    fflush( stdout );
}

namespace reef {

RemoteObject::RemoteObject() : _numFacets( 0 ), _componentType( 0 )
{
    // empty constructor
}

RemoteObject::~RemoteObject()
{
    // empty destructor
}
    
void RemoteObject::setComponent( co::IComponent* component )
{    
    assert( _componentType == 0 && _numFacets == 0 );
    
	_componentType = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _componentType->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
	_facets = new co::IService*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
		/*
         To avoid having exceptions raised here by getReflector(), we should
         check all interface reflectors before instantiating the component.
		 */
		assert( _numFacets == i );
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
		assert( _numFacets == i + 1 );
	}
 
}

co::IComponent* RemoteObject::getComponent()
{
    return _componentType;
}

co::IService* RemoteObject::getServiceAt( co::IPort* port )
{
    co::ICompositeType* owner = port->getOwner();
    if( owner == _componentType )
    {
        int portIndex = port->getIndex();
        assert( portIndex <= _numFacets );
        return _facets[portIndex];
    }
    
    return reef::RemoteObject_Base::getServiceAt( port );
}

void RemoteObject::setServiceAt( co::IPort* receptacle, co::IService* instance )
{
    // empty: setting remote services not supported yet
}

co::IPort* RemoteObject::dynamicGetFacet( co::int32 dynFacetId )
{
    co::IService* service = _facets[dynFacetId];
    printServiceInfo( service, "Dynamic Get Facet!" );
    return service->getFacet();
}

const co::Any& RemoteObject::dynamicGetField( co::int32 dynFacetId, co::IField* field )
{
    co::IService* service = _facets[dynFacetId];
    printServiceInfo( service, "Dynamic Get FIELD!" );
    static co::Any dummy;
    return dummy;
}

const co::Any& RemoteObject::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, co::Range<co::Any const> args )
{
    co::IService* service = _facets[dynFacetId];
    printServiceInfo( service, "Dynamic INVOKE!" );
    static co::Any dummy;
    return dummy;
}

co::int32 RemoteObject::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}

void RemoteObject::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    co::IService* service = _facets[dynFacetId];
    printServiceInfo( service, "Dynamic Set FIELD!" );
}

CORAL_EXPORT_COMPONENT( RemoteObject, RemoteObject );
    
} // namespace reef
