#include "RemoteObject.h"

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>

namespace reef {

RemoteObject::RemoteObject()
{
    // empty
}
    
RemoteObject::RemoteObject( co::IComponent* component, Channel* channel ) : _numFacets( 0 )
{
    setComponent( component );
    channel->newInstance( component->getFullName() );
    _channel = channel;
}

RemoteObject::~RemoteObject()
{
    delete _channel;
    
    for( std::vector<co::IService*>::iterator it = _facets.begin(); it != _facets.end(); it++ )
    {
        delete (*it);
    }
}
    
void RemoteObject::setComponent( co::IComponent* component )
{    
	_componentType = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _componentType->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets.resize( numFacets );
	for( int i = 0; i < numFacets; ++i )
	{
		assert( _numFacets >= i );
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
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
    return service->getFacet();
}

const co::Any& RemoteObject::dynamicGetField( co::int32 dynFacetId, co::IField* field )
{
    _channel->getField( dynFacetId, field, _resultBuffer );
    return _resultBuffer;
}

const co::Any& RemoteObject::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, co::Range<co::Any const> args )
{
	if( !method->getReturnType() )
		_channel->sendCall( dynFacetId, method, args );
	else
		_channel->call( dynFacetId, method, args, _resultBuffer );

    return _resultBuffer;
}

co::int32 RemoteObject::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}

void RemoteObject::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    _channel->setField( dynFacetId, field, value );
}

void RemoteObject::onReferenceReturned( co::IMethod* method )
{

}

void RemoteObject::checkReferenceParams( co::IMethod* method, co::Range<co::Any const> args )
{
	co::Range<co::IParameter* const> params = method->getParameters();
	for( int i = 0; params; params.popFirst(), i++ )
	{
		
	}
}

CORAL_EXPORT_COMPONENT( RemoteObject, RemoteObject );
    
} // namespace reef
