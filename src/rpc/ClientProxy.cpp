#include "ClientProxy.h"

#include "Node.h"
#include "Requestor.h"

#include <rpc/IConnector.h>

#include <co/Log.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>
#include <co/IRecordType.h>
#include <co/IParameter.h>

#include <map>

namespace rpc {

void* ClientProxy::_classPtr = 0;
    
ClientProxy::ClientProxy()
{
}
    
ClientProxy::ClientProxy( Requestor* requestor, co::IComponent* component, co::int32 instanceID ) : 
    _requestor( requestor ), _instanceID( instanceID ), _numFacets( 0 )
{
    _classPtr = *reinterpret_cast<void**>( this );
    setComponent( component );
}
    
ClientProxy::~ClientProxy()
{   
    for( int i = 0; i < _numFacets; i++ )
    {
        delete _facets[i];
    }
    delete [] _interfaces;
    delete [] _facets;
}
    
void ClientProxy::setComponent( co::IComponent* component )
{    
	_component = component;
    
	// create proxy interfaces for our facets
	co::TSlice<co::IPort*> facets = _component->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets = new co::IService*[numFacets];
    _interfaces = new co::IInterface*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
        _interfaces[i] = facets[i]->getType();
	}
}

co::IComponent* ClientProxy::getComponent()
{
    return _component;
}
    
co::IService* ClientProxy::getServiceAt( co::IPort* port )
{
    co::ICompositeType* owner = port->getOwner();
    if( owner == _component )
    {
        int portIndex = port->getIndex();
        assert( portIndex <= _numFacets );
        return _facets[portIndex];
    }
    
    return rpc::ClientProxy_Base::getServiceAt( port );
}

void ClientProxy::setServiceAt( co::IPort* receptacle, co::IService* instance )
{
    // empty: setting remote services not supported yet
}

co::IPort* ClientProxy::dynamicGetFacet( co::int32 cookie )
{
    return _component->getFacets()[cookie];
}
        
void ClientProxy::dynamicGetField( co::int32 dynFacetId, co::IField* field, 
                                            const co::Any& value )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], field->getOwner() );

    MemberOwner mo( _instanceID, dynFacetId, depth );
    
    _requestor->requestGetField( mo, field, value );
}
    
void ClientProxy::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], field->getOwner() );
    
    MemberOwner mo( _instanceID, dynFacetId, depth );
    _requestor->requestSetField( mo, field, value );
}

static void printRet( co::IRecordType* rt, const co::Any instance )
{
    co::IReflector* refl = rt->getReflector();
    for( co::TSlice<co::IField*> fields = rt->getFields(); fields; fields.popFirst() )
    {
        co::IField* field = fields.getFirst();
        co::IType* fieldType = field->getType();
        
        co::AnyValue av; co::Any fieldAny( av );
        
        refl->getField( instance, fields.getFirst(), fieldAny );
        
        CORAL_LOG(INFO) << "Field of type " << fieldType->getName() << " got value of type " << fieldAny.asIn().getType()->getName();
        
        if( fieldAny.getKind() == co::TK_ANY && fieldAny.asIn().getKind() == co::TK_INT32 )
        {
            CORAL_LOG(INFO) << "Int inside any has value of " << fieldAny.asIn().get<co::int32>();
        }
        
        if( fieldAny.asIn().getKind() == co::TK_STRUCT )
        {
            CORAL_LOG(INFO) << "Recursively printing field struct";
            printRet( co::cast<co::IRecordType>( fieldType ), fieldAny );
        }
    }
}
    
void ClientProxy::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, 
                                           co::Slice<co::Any> args, const co::Any& result )
{
    co::int32 depth = findDepth( _interfaces[dynFacetId], method->getOwner() );
    
    MemberOwner mo( _instanceID, dynFacetId, depth );
    
    co::IType* returnType = method->getReturnType();
    
    if( !returnType )
        _requestor->requestAsynchCall( mo, method, args );
    else
        _requestor->requestSynchCall( mo, method, args, result );
    
    CORAL_LOG(INFO) << "Returning a " << result.asIn().getType()->getName() << " of kind " << result.asIn().getKind();
    
    co::IType* elementsType = co::cast<co::IArray>( result.getType() )->getElementType();
    co::int32 size = result.getCount();
    for( int i = 0; i < size; i++ )
    {
        co::IRecordType* rt = co::cast<co::IRecordType>( elementsType );
        CORAL_LOG(INFO) << "Printing " << result.getType()->getName() << " " << i;
        printRet( rt, result[i] );
    }
}

co::int32 ClientProxy::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}

    // ------ rpc.IInstanceInfo Methods ------ //
    
co::int32 ClientProxy::getInstanceId()
{
    return _instanceID;
}

Requestor* ClientProxy::getRequestor()
{
    return _requestor.get();
}

co::int32 ClientProxy::findDepth( co::IInterface* facet, co::ICompositeType* memberOwner )
{
    if( memberOwner == facet )
        return -1;

    co::TSlice<co::IInterface*> superTypes( facet->getSuperTypes() );
    for( int i = 0; superTypes; superTypes.popFirst(), i++ )
    {
        if( superTypes.getFirst() == memberOwner )
            return i;
    }
    return -2;
}
    
CORAL_EXPORT_COMPONENT( ClientProxy, ClientProxy );
 
} // namespace rpc
