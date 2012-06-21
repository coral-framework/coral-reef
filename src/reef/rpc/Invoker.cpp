#include "Invoker.h"

#include "Node.h"
#include "Requestor.h"
#include "ClientProxy.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "InstanceContainer.h"
#include "ServerRequestHandler.h"

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


Invoker::Invoker( Node* node, ServerRequestHandler* srh, RequestorManager* requestorMan ) : 
                _node( node ), _srh( srh ), _requestorMan( requestorMan )
{
}
    
Invoker::~Invoker()
{
}

void Invoker::dispatchInvocation( const std::string& invocation )
{
    co::int32 destInstanceId;
    Demarshaller::MsgType type;
    bool hasReturn;
    _demarshaller.setMarshalledRequest( invocation, type, destInstanceId, hasReturn );
    
    std::string returnValue;
    
    if( destInstanceId == 0 )
        invokeManager( _demarshaller, type, hasReturn, returnValue );
    else
        invokeInstance( _demarshaller, destInstanceId, hasReturn, returnValue );
    
    if( hasReturn )
        _srh->reply( returnValue );
}

void Invoker::invokeManager( Demarshaller& demarshaller, Demarshaller::MsgType type, bool isSynch, 
                   std::string& returned )
{
    co::int32 returnID;
    std::string lesseeEndpoint;
    
    switch( type )
    {
        case Demarshaller::NEW_INST:
        {
            std::string componentName;
            _demarshaller.demarshalNewInstance( componentName, lesseeEndpoint );
            returnID = _instanceMan->newInstance( componentName, lesseeEndpoint );
            break;
        }
        case Demarshaller::ACCESS_INST:
        {
            co::int32 instanceID;
            bool increment;
            
            // REMOTINGERROR: if no instance found with the id
            _demarshaller.demarshalAccessInstance( instanceID, increment, lesseeEndpoint );
            if( increment )
                _instanceMan->createLease( instanceID, lesseeEndpoint );
            else
                _instanceMan->cancelLease( instanceID, lesseeEndpoint );
            
            break;
        }
        case Demarshaller::FIND_INST:
        {
            std::string key;
            _demarshaller.demarshalFindInstance( key, lesseeEndpoint );
            returnID = _instanceMan->findInstance( key, lesseeEndpoint );
            break;
        }
        default:
            assert( false );
    }  

}
    
void Invoker::invokeInstance( Demarshaller& demarshaller, co::int32 instanceID, bool isSynch, 
                             std::string& returned )
{    
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::Any parameter;
    co::int32 depth;
    std::string caller;
    
    demarshaller.beginDemarshallingCall( facetIdx, memberIdx, depth, caller );
    
    InstanceContainer* container = _instanceMan->getInstance( instanceID );
    
    co::IService* facet = container->getCachedService( facetIdx );
    
    co::IInterface* memberOwner = facet->getInterface();
    
    // if method is inherited
    if( depth != -1 )
        memberOwner = memberOwner->getSuperTypes()[depth];
        
    co::IReflector* reflector = memberOwner->getReflector();
    co::IMember* member = memberOwner->getMembers()[memberIdx];
    co::MemberKind kind = member->getKind();
    
    if( kind == co::MK_METHOD )
    {
        onMethod( demarshaller, facet, co::cast<co::IMethod>( member ), reflector, parameter );
        if( !isSynch )
            return;
    }   
    else   
    {
        if( !isSynch )
        {
            onSetField( demarshaller, facet, co::cast<co::IField>( member ), reflector );
            return;
        }
        else
        {
            onGetField( demarshaller, facet, co::cast<co::IField>( member ), reflector, parameter );
        }
    }
   
    // Marshals the return from the call
    if( parameter.getKind() != co::TK_INTERFACE )
        _marshaller.marshalValueType( parameter, returned );
    else
        onInterfaceReturned( parameter.get<co::IService*>(), caller, returned );
}

void Invoker::onMethod( Demarshaller& demarshaller, co::IService* facet, co::IMethod* method, 
                           co::IReflector* refl, co::Any& returned )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    std::vector<co::Any> args;
    co::Range<co::IParameter* const> params = method->getParameters(); 
    
    size_t size = params.getSize();
    args.resize( size );
    for( int i = 0; i < size; i++ )
    {
        demarshalParameter( demarshaller, params[i]->getType(), args[i], tempReferences );
    }

    refl->invoke( facet, method, args, returned );
}
    
void Invoker::onGetField( Demarshaller& demarshaller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl, co::Any& returned )
{
    refl->getField( facet, field, returned );
}

void Invoker::onSetField(  Demarshaller& demarshaller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    co::Any value;
    demarshalParameter( demarshaller, field->getType(), value, tempReferences );
    
    refl->setField( facet, field, value );
}
    
void Invoker::demarshalParameter( Demarshaller& demarshaller, co::IType* paramType, co::Any& param, 
                         co::RefVector<co::IObject>& tempRefs )
{
    if( paramType->getKind() != co::TK_INTERFACE )
    {
        demarshaller.demarshalValueParam( param, paramType );
        return;
    }
    
    co::int32 instanceId;
    co::int32 facetIdx;
    Demarshaller::RefOwner owner;
    std::string instanceType;
    std::string ownerAddress;
    
    demarshaller.demarshalReferenceParam( instanceId, facetIdx, owner, instanceType, ownerAddress );
    co::IObject* instance;
    switch( owner )
    {
        case Demarshaller::RECEIVER:
            instance = _node->getInstance( instanceId );
            break;
        case Demarshaller::LOCAL:
        case Demarshaller::ANOTHER:
            Requestor* req = _requestorMan->getOrCreateRequestor( ownerAddress );
            instance = req->getOrCreateProxy( instanceId, instanceType );
            break;
    }
    tempRefs.push_back( instance ); // TODO: remove
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[facetIdx];
    co::IService* service = instance->getServiceAt( port );
    param.set<co::IService*>( service );
}

void Invoker::onInterfaceReturned( co::IService* returned, std::string& caller, 
                                      std::string& marshalledReturn )
{
    co::IObject* provider = returned->getProvider();
    std::string providerType = provider->getComponent()->getFullName();
    co::int32 facetIdx = returned->getFacet()->getIndex();
    co::int32 instanceId;
    
    if( !ClientProxy::isClientProxy( provider ) )
    {
        instanceId = _node->publishAnonymousInstance( provider, caller );
        _marshaller.marshalReferenceType( instanceId, facetIdx, Marshaller::RefOwner::LOCAL, 
							marshalledReturn, &providerType, &_node->getPublicAddress() );
    }
    else
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        instanceId = providerCP->getInstanceId();        
        Requestor* requestor = providerCP->getRequestor();
        
        if( requestor->getEndpoint() == caller ) // Receiver
        {
            _marshaller.addReferenceParam( instanceId, facetIdx, Marshaller::RECEIVER );
        }
        else
        {
            requestor->requestLease( instanceId, caller );
            std::string othersEndpoint( requestor->getEndpoint() );
            _marshaller.marshalReferenceType( instanceId, facetIdx, Marshaller::ANOTHER, 
                                             marshalledReturn, &providerType, &othersEndpoint );
        }

    }
}

}
    
} // namespace reef


