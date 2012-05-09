#include "Invoker.h"

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

namespace reef {
namespace rpc {


Invoker::Invoker( Node* node, co::IObject* object ) : _node( node )
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
    
Invoker::~Invoker()
{

}
    
void Invoker::onCallOrField( Unmarshaller& unmarshaller, co::Any* retValue )
{
    co::int32 facetIdx;
    co::int32 memberIdx;
    unmarshaller.beginUnmarshallingCall( facetIdx, memberIdx );
    
    if( !_openedServices[facetIdx] ) // if already used before access directly
		onServiceFirstAccess( facetIdx );

    co::IMember* member = _openedInterfaces[facetIdx]->getMembers()[memberIdx];
    co::MemberKind kind = member->getKind();
    
    if( kind == co::MK_METHOD )
    {
        onMethod( unmarshaller, facetIdx, co::cast<co::IMethod>( member ), retValue );
    }
    else if( kind == co::MK_FIELD )
    {
        onField( unmarshaller, facetIdx, co::cast<co::IField>( member ), retValue );
    }
}

void Invoker::onMethod( Unmarshaller& unmarshaller, co::int32 facetIdx, co::IMethod* method, 
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
        onGetParam( unmarshaller, params[i]->getType(), args[i], tempReferences );
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
    
void Invoker::onField( Unmarshaller& unmarshaller, co::int32 facetIdx, co::IField* field, co::Any* retValue )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    if( !retValue )
    {
        co::Any value;
        onGetParam( unmarshaller, field->getType(), value, tempReferences );
    
        _openedReflectors[facetIdx]->setField( _openedServices[facetIdx], field, value );
    }
    else
    {
        _openedReflectors[facetIdx]->getField( _openedServices[facetIdx], field, *retValue );
    }
}
 
void Invoker::onGetParam( Unmarshaller& unmarshaller, co::IType* paramType, co::Any& param, 
                         co::RefVector<co::IObject>& tempRefs )
{
    if( paramType->getKind() != co::TK_INTERFACE )
    {
        unmarshaller.unmarshalValueParam( param, paramType );
        return;
    }
    
    co::int32 instanceID;
    co::int32 facetIdx;
    Unmarshaller::RefOwner owner;
    std::string instanceType;
    std::string ownerAddress;
    
    unmarshaller.unmarshalRefParam( instanceID, facetIdx, owner, instanceType, ownerAddress );
    co::IObject* instance;
    switch( owner )
    {
        case Unmarshaller::RefOwner::RECEIVER:
            instance = _node->getInstance( instanceID );
            break;
        case Unmarshaller::RefOwner::LOCAL:
        case Unmarshaller::RefOwner::ANOTHER:
            instance = _node->getRemoteInstance( instanceType, instanceID, 
                                                     ownerAddress );
            break;
    }
    tempRefs.push_back( instance ); // TODO: remove
    
    co::Range<co::IPort* const> ports = _object->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    co::IService* service = instance->getServiceAt( port );
    param.set<co::IService*>( service );
}
    
void Invoker::onServiceFirstAccess( co::int32 serviceId )
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
    
}
    
} // namespace reef


