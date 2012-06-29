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


Invoker::Invoker( InstanceManager* instanceMan, ServerRequestHandler* srh, 
                 RequestorManager* requestorMan ) : _instanceMan( instanceMan ),
                _srh( srh ), _requestorMan( requestorMan )
{
}
    
Invoker::~Invoker()
{
}

void Invoker::dispatchInvocation( const std::string& invocationData )
{
    MessageType msgType = _demarshaller.demarshal( invocationData );
    
    std::string returnValue;
    
    if( msgType != INVOCATION )
        invokeManager( _demarshaller, msgType, returnValue );
    else
        invokeInstance( _demarshaller, returnValue );
    
    if( !returnValue.empty() )
        _srh->reply( returnValue );
}

void Invoker::invokeManager( Demarshaller& demarshaller, MessageType msgType, 
                            std::string& returnValue )
{
    co::int32 returnID;
    std::string requesterEndpoint;
    
    switch( msgType )
    {
        case REQUEST_NEW:
        {
            std::string instanceType;
            _demarshaller.getNew( requesterEndpoint, instanceType );
            co::IObject* instance = co::newInstance( instanceType );
            returnID = _instanceMan->addInstance( instance, requesterEndpoint );
            break;
        }
        case REQUEST_LOOKUP:
        {
            std::string key;
            _demarshaller.getLookup( requesterEndpoint, key );
            returnID = _instanceMan->findInstance( key, requesterEndpoint );
            break;
        }
        case REQUEST_LEASE:
        {
            co::int32 instanceID;
            
            // REMOTINGERROR: if no instance found with the id
            _demarshaller.getLease( requesterEndpoint, instanceID );
            
            _instanceMan->createLease( instanceID, requesterEndpoint );
            return;
        }
        case REQUEST_CANCEL_LEASE:
        {
            co::int32 instanceID;
            
            // REMOTINGERROR: if no instance found with the id
            _demarshaller.getLease( requesterEndpoint, instanceID );
            
            _instanceMan->cancelLease( instanceID, requesterEndpoint );
            return;
        }
        default:
            assert( false );
    }
    
    _marshaller.marshalIntReturn( returnID , returnValue );
}
    
void Invoker::invokeInstance( Demarshaller& demarshaller, std::string& returnMsg )
{    
    std::string senderEndpoint;
    InvocationDetails details;
    
    ParameterPuller& puller = demarshaller.getInvocation( senderEndpoint, details );
    
    InstanceContainer* container = _instanceMan->getInstance( details.instanceID );
    
    co::IService* facet = container->getCachedService( details.facetIdx );
    
    co::IInterface* memberOwner = facet->getInterface();
    
    // if method is inherited
    if( details.typeDepth != -1 )
        memberOwner = memberOwner->getSuperTypes()[details.typeDepth];
        
    co::IReflector* reflector = memberOwner->getReflector();
    co::IMember* member = memberOwner->getMembers()[details.memberIdx];
    co::MemberKind kind = member->getKind();
    
    co::Any returnAny;
    
    if( kind == co::MK_METHOD )
    {
        onMethod( puller, facet, co::cast<co::IMethod>( member ), reflector, returnAny );
        if( !details.hasReturn )
            return;
    }   
    else   
    {
        if( !details.hasReturn )
        {
            onSetField( puller, facet, co::cast<co::IField>( member ), reflector );
            return;
        }
        else
        {
            onGetField( facet, co::cast<co::IField>( member ), reflector, returnAny );
        }
    }
   
    // Marshals the return from the call
    if( returnAny.getKind() != co::TK_INTERFACE )
    {
        _marshaller.marshalValueTypeReturn( returnAny, returnMsg );
    }
    else
    {
        ReferenceType refType;
        getRefTypeInfo( returnAny.get<co::IService*>(), senderEndpoint, refType );
        _marshaller.marshalRefTypeReturn( refType, returnMsg );
    }
}

void Invoker::onMethod( ParameterPuller& puller, co::IService* facet, co::IMethod* method, 
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
        co::IType* paramType = params[i]->getType();
        if( paramType->getKind() != co::TK_INTERFACE )
        {
            puller.pullValue( paramType, args[i] );
        }
        else
        {
            ReferenceType refTypeInfo;
            puller.pullReference( refTypeInfo );
            getRefType( refTypeInfo, args[i], tempReferences );
        }
    }

    refl->invoke( facet, method, args, returned );
}
    
void Invoker::onGetField( co::IService* facet, co::IField* field, 
                             co::IReflector* refl, co::Any& returned )
{
    refl->getField( facet, field, returned );
}

void Invoker::onSetField( ParameterPuller& puller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl )
{
    // TODO: remove this and maky the co::any's reference the objects themselves
    co::RefVector<co::IObject> tempReferences;
    
    co::Any value;
    
    co::IType* fieldType = field->getType();
    
    if( fieldType->getKind() != co::TK_INTERFACE )
    {
        puller.pullValue( fieldType, value );
    }
    else
    {
        ReferenceType refTypeInfo;
        puller.pullReference( refTypeInfo );
        getRefType( refTypeInfo, value, tempReferences );
    }
    
    refl->setField( facet, field, value );
}
    
void Invoker::getRefType( ReferenceType& refTypeInfo, co::Any& param, 
                             co::RefVector<co::IObject>& tempRefs )
{
    co::IObject* instance;
    switch( refTypeInfo.owner )
    {
        case OWNER_RECEIVER:
            instance = _instanceMan->getInstance( refTypeInfo.instanceID )->getInstance();
            break;
        case OWNER_SENDER:
        case OWNER_ANOTHER:
            Requestor* req = _requestorMan->getOrCreateRequestor( refTypeInfo.ownerEndpoint );
            instance = req->getOrCreateProxy( refTypeInfo.instanceID, refTypeInfo.instanceType );
            break;
    }
    tempRefs.push_back( instance ); // TODO: remove
    
    co::Range<co::IPort* const> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[refTypeInfo.facetIdx];
    co::IService* service = instance->getServiceAt( port );
    param.set<co::IService*>( service );
}

void Invoker::getRefTypeInfo( co::IService* service, std::string& senderEndpoint, 
                                  ReferenceType& refType )
{
    co::IObject* provider = service->getProvider();
    
    refType.instanceType = provider->getComponent()->getFullName();
    
    refType.facetIdx = service->getFacet()->getIndex();
    
    if( !ClientProxy::isClientProxy( provider ) )
    {
        refType.instanceID = _instanceMan->addInstance( provider, "self" );
        refType.owner = OWNER_SENDER;
        refType.ownerEndpoint = _srh->getPublicEndpoint();
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        ClientProxy* providerCP = static_cast<ClientProxy*>( provider );
        
        refType.instanceID = providerCP->getInstanceId();
        
        Requestor* requestor = providerCP->getRequestor();
        
        if( requestor->getEndpoint() == senderEndpoint ) // Receiver
        {
            refType.owner = OWNER_RECEIVER;
            refType.ownerEndpoint = "";
        }
        else
        {
            requestor->requestLease( refType.instanceID, senderEndpoint );
            refType.owner = OWNER_ANOTHER;
            refType.ownerEndpoint = requestor->getEndpoint();
        }
    }
}

}
    
} // namespace reef


