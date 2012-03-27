#include "RemoteObject.h"

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

namespace reef {

    
RemoteObject::RemoteObject()
{
    // empty
}
    
RemoteObject::RemoteObject( 
        co::IComponent* component, Encoder* encoder, ServerNode* serverNode ) : 
        _numFacets( 0 ), _serverNode( serverNode )
{
    setComponent( component );
    encoder->newInstance( component->getFullName() );
    _encoder = encoder;
    
    // necessary to separate regular from remote objects
    _remObjFingerprint = *reinterpret_cast<void**>( this );
}

RemoteObject::~RemoteObject()
{
    delete _encoder;
    
    for( int i = 0; i < _numFacets; i++ )
    {
        delete _facets[i];
    }
    delete [] _facets;
}
    
void RemoteObject::setComponent( co::IComponent* component )
{    
	_componentType = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _componentType->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets = new co::IService*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
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
    _encoder->getField( dynFacetId, field, _resultBuffer );
    return _resultBuffer;
}

const co::Any& RemoteObject::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, co::Range<co::Any const> args )
{
	if( !method->getReturnType() )
		_encoder->sendCall( dynFacetId, method, args );
	else
		_encoder->call( dynFacetId, method, args, _resultBuffer );

    return _resultBuffer;
}

co::int32 RemoteObject::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}

void RemoteObject::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    _encoder->setField( dynFacetId, field, value );
}

void RemoteObject::onLocalObjParam( co::IService* param )
{
    
}

void RemoteObject::onRemoteObjParam( co::IService* param )
{
    
}
    
void RemoteObject::onReferenceReturned( co::IMethod* method )
{

}

void RemoteObject::checkReferenceParams( co::IMethod* method, co::Range<co::Any const> args )
{
	co::Range<co::IParameter* const> params = method->getParameters();
	for( ; params; params.popFirst(), args.popFirst() )
	{
		if( params.getFirst()->getType()->getKind() == co::TK_INTERFACE )
        {
            co::IService* param = args.getFirst().get<co::IService*>();
            if( isLocalObject( param->getProvider() ) )
               onLocalObjParam( param );
            else
               onRemoteObjParam( param );
            
        }
	}
}

CORAL_EXPORT_COMPONENT( RemoteObject, RemoteObject );
    
} // namespace reef
