#include "Servant.h"

#include <co/IPort.h>
#include <co/IMethod.h>
#include <co/IMember.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <string>
#include <iostream>

namespace reef
{

Servant::Servant( co::IObject* object )
{
    if( object )
    {
        _object = object;
        co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
        _openedServices.resize( ports.getSize() );
        _openedReflectors.resize( ports.getSize() );
    }
}
    
Servant::~Servant()
{

}
    
void Servant::onCall( Decoder& decoder, bool hasReturn, co::Any* retValue )
{
    co::int32 facetIdx;
    co::int32 memberIdx;
    decoder.beginDecodingCallMsg( facetIdx, memberIdx );
    
    if( !_openedServices[facetIdx] ) // if already used before access directly
		onServiceFirstAccess( facetIdx );

    co::IMember* member = _openedInterfaces[facetIdx]->getMembers()[memberIdx];
    co::MemberKind kind = member->getKind();
    if( kind == co::MK_METHOD )
    {
        onCallMethod( facetIdx, co::cast<co::IMethod>( member ), decoder, retValue );
    }
    else if( kind == co::MK_FIELD )
    {
        if( hasReturn )
            onGetField( facetIdx, co::cast<co::IField>( member ), decoder, retValue );
        else
            onSetField( facetIdx, co::cast<co::IField>( member ), decoder );
    }
}

void Servant::onCallMethod( Decoder& decoder, co::int32 facetIdx, co::IMethod* method, 
                           co::Any* retValue )
{
    std::vector<co::Any> anyArgs;
    co::Range<co::IParameter* const> params = method->getParameters(); 
    
    size_t size = params.getSize();
    anyArgs.resize( size );
    for( int i = 0; i < size; i++ )
    {
        co::IType* paramType = params[i]->getType();
        if( paramType->getKind() != co::TK_INTERFACE )
            decoder.getValueParam( anyArgs[i], paramType );
        else
            onRefParam( decoder, anyArgs[i] );
    }

}
void Servant::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
{
	if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
    
    co::Any dummy;
		
	_openedReflectors[serviceId]->invoke( _openedServices[serviceId], method, args, dummy );
}
    
void Servant::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
{
    if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
		
	_openedReflectors[serviceId]->invoke( _openedServices[serviceId], method, args, result );		
}
    
void Servant::getField( co::int32 serviceId, co::IField* field, co::Any& result )
{
    if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
    
	_openedReflectors[serviceId]->getField( _openedServices[serviceId], field, result );
}
    
void Servant::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
{
    if( !_openedServices[serviceId] ) // if already used before access directly
		onServiceFirstAccess( serviceId );
    
	_openedReflectors[serviceId]->setField( _openedServices[serviceId], field, value );
}

void Servant::onServiceFirstAccess( co::int32 serviceId )
{
	co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[serviceId];
    assert( port->getIsFacet() );
    
    co::IService* service = _object->getServiceAt( port );

    co::IInterface* itf = service->getInterface();

    co::IReflector* reflector = itf->getReflector();

	// saving for easier later access
	_openedServices[serviceId] = service;
    _openedInterfaces[serviceId] = itf;
    _openedReflectors[serviceId] = reflector;
}
    
} // namespace reef


