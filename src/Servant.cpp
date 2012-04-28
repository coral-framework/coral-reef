#include "Servant.h"

#include "Node.h"
#include "Node.h"

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IMember.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <string>
#include <iostream>

namespace reef
{

Servant::Servant( Node* node, co::IObject* object ) : _node( node )
{
    if( object )
    {
        _object = object;
        _component = _object->getComponent();
        co::Range<co::IPort* const> ports = _component->getFacets();
        co::int32 numPorts = ports.getSize();
        _openedServices.resize( numPorts );
        _openedInterfaces.resize( numPorts );
        _openedReflectors.resize( numPorts );
    }
}
    
Servant::~Servant()
{

}
    
void Servant::onCallOrField( Decoder& decoder, co::Any* retValue )
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
        onMethod( decoder, facetIdx, co::cast<co::IMethod>( member ), retValue );
    }
    else if( kind == co::MK_FIELD )
    {
        onField( decoder, facetIdx, co::cast<co::IField>( member ), retValue );
    }
}

void Servant::onMethod( Decoder& decoder, co::int32 facetIdx, co::IMethod* method, 
                           co::Any* retValue )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    std::vector<co::Any> args;
    co::Range<co::IParameter* const> params = method->getParameters(); 
    
    size_t size = params.getSize();
    args.resize( size );
    for( int i = 0; i < size; i++ )
    {
        onGetParam( decoder, params[i]->getType(), args[i], tempReferences );
    }

    if( !retValue )
    {
        co::Any dummy;
        _openedReflectors[facetIdx]->invoke( _openedServices[facetIdx], method, args, dummy );
    }
    else
    {
        _openedReflectors[facetIdx]->invoke( _openedServices[facetIdx], method, args, *retValue );
    }
}
    
void Servant::onField( Decoder& decoder, co::int32 facetIdx, co::IField* field, co::Any* retValue )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    if( !retValue )
    {
        co::Any value;
        onGetParam( decoder, field->getType(), value, tempReferences );
    
        _openedReflectors[facetIdx]->setField( _openedServices[facetIdx], field, value );
    }
    else
    {
        _openedReflectors[facetIdx]->getField( _openedServices[facetIdx], field, *retValue );
    }
}
 
void Servant::onGetParam( Decoder& decoder, co::IType* paramType, co::Any& param, 
                         co::RefVector<co::IObject>& tempRefs )
{
    if( paramType->getKind() != co::TK_INTERFACE )
    {
        decoder.getValueParam( param, paramType );
        return;
    }
    
    co::int32 instanceID;
    co::int32 facetIdx;
    Decoder::RefOwner owner;
    std::string instanceType;
    std::string ownerAddress;
    
    decoder.getRefParam( instanceID, facetIdx, owner, instanceType, ownerAddress );
    co::IObject* instance;
    switch( owner )
    {
        case Decoder::RefOwner::RECEIVER:
            instance = _node->getInstance( instanceID );
            break;
        case Decoder::RefOwner::LOCAL:
        case Decoder::RefOwner::ANOTHER:
            instance = _node->getRemoteInstance( instanceType, instanceID, 
                                                     ownerAddress );
            break;
    }
    tempRefs.push_back( instance ); // TODO: remove
    
    co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    param.set<co::IService*>( instance->getServiceAt( port ) );
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


