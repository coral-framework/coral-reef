#include "Invoker.h"

#include "Node.h"
#include "Requestor.h"
#include "ClientProxy.h"
#include "BarrierManager.h"
#include "InstanceManager.h"
#include "RequestorManager.h"
#include "InstanceContainer.h"
#include "ServerRequestHandler.h"

#include <rpc/RemotingException.h>

#include <co/Log.h>
#include <co/IPort.h>
#include <co/IType.h>
#include <co/IField.h>
#include <co/ISystem.h>
#include <co/IMethod.h>
#include <co/IMember.h>
#include <co/Exception.h>
#include <co/IReflector.h>
#include <co/IParameter.h>
#include <co/ITypeManager.h>

#include <string>
#include <iostream>
#include <sstream>

namespace rpc {

    
Invoker::Invoker( InstanceManager* instanceMan, BarrierManager* barrierMan, ServerRequestHandler* srh, 
            RequestorManager* requestorMan ) : _instanceMan( instanceMan ), _barrierMan( barrierMan ),
                _srh( srh ), _requestorMan( requestorMan ), _barrierUp( false )
{
}
    
Invoker::~Invoker()
{
}

void Invoker::dispatchInvocation( const std::string& invocationData )
{    
    std::string returnValue;
    
    try
    {
        MessageType msgType = _demarshaller.demarshal( invocationData );
        
        if( msgType != INVOCATION )
            invokeManagement( _demarshaller, msgType, returnValue );
        else
            invokeInstance( _demarshaller, returnValue );
    }
    catch( RemotingException& e )
    {
        CORAL_LOG( ERROR ) << "Request triggered a remoting exception: " << e.what();
        _marshaller.marshalException( EX_REMOTING, e.getTypeName(), e.what(), returnValue );
    }
    catch( co::Exception& e )
    {
        _marshaller.marshalException( EX_CORAL, e.getTypeName(), e.what(), returnValue );
    }
    catch( std::exception& e )
    {
        _marshaller.marshalException( EX_STD, "std", e.what(), returnValue );
    }
    
    if( !returnValue.empty() )
        _srh->reply( returnValue );
}

void Invoker::hitBarrier()
{
    while( !_barrierUp ) // tried to enter the barrier before its raising
        _srh->react();
    
    _barrierCreator->requestBarrierHit();
    
    while( _barrierUp )
        _srh->react();
}
    
void Invoker::invokeManagement( Demarshaller& demarshaller, MessageType msgType, 
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
            
            co::IType* type = co::getSystem()->getTypes()->findType( instanceType );
            
            if( !type )
                CORAL_THROW( RemotingException, "Unknown component type: " << instanceType );
            
            CORAL_DLOG( INFO ) << "Received request for New instance of type " << type->getName() << 
                        " from: " <<  requesterEndpoint;

            co::IObject* instance = type->getReflector()->newInstance();
            returnID = _instanceMan->addInstance( instance, requesterEndpoint );
            break;
        }
        case REQUEST_LOOKUP:
        {
            std::string key;
            std::string instanceType;
            _demarshaller.getLookup( requesterEndpoint, key, instanceType );
            
            CORAL_DLOG( INFO ) << "Received request for Looking up key " << key << " from: " << requesterEndpoint;
            
            returnID = _instanceMan->findInstance( key, instanceType, requesterEndpoint );
            break;
        }
        case REQUEST_LEASE:
        {
            co::int32 instanceID;
            
            _demarshaller.getLease( requesterEndpoint, instanceID );
            
            CORAL_DLOG( INFO ) << "Received request for lease to " << instanceID << " from: " << requesterEndpoint;
            
            if( !_instanceMan->getInstance( instanceID ) )
                CORAL_THROW( RemotingException, "No such instance of id: " << instanceID );
            
            _instanceMan->createLease( instanceID, requesterEndpoint );
            return;
        }
        case REQUEST_CANCEL_LEASE:
        {
            co::int32 instanceID;
            
            _demarshaller.getLease( requesterEndpoint, instanceID );
            
            CORAL_DLOG( INFO ) << "Received request for cancel lease to " << instanceID << " from: " << requesterEndpoint;
            
            if( !_instanceMan->getInstance( instanceID ) )
                CORAL_THROW( RemotingException, "No such instance of id: " << instanceID );
            
            _instanceMan->cancelLease( instanceID, requesterEndpoint );
            return;
        }
        case BARRIER_UP:
        {
            _barrierUp = true;
            
            _demarshaller.getBarrierCreator( requesterEndpoint );
            
            CORAL_DLOG( INFO ) << "Received barrier up signal from " << requesterEndpoint;
            
            _barrierCreator = _requestorMan->getOrCreateRequestor( requesterEndpoint );
            return;
        }
        case BARRIER_HIT:
        {
            CORAL_DLOG( INFO ) << "Received a barrier hit signal";            
            _barrierMan->onBarrierHit();
            return;
        }
        case BARRIER_DOWN:
        {
            CORAL_DLOG( INFO ) << "Received a barrier down signal";
            
            _barrierUp = false;
            return;
        }
        default:
            assert( false );
    }
    
    _marshaller.marshalIntReturn( returnID , returnValue );
}
    
void Invoker::invokeInstance( Demarshaller& demarshaller, std::string& returnMsg )
{    
    // ------ Get invocation meta data and check for inconsistencies first ------ //
    std::string senderEndpoint;
    InvocationDetails details;
    
    ParameterPuller& puller = demarshaller.getInvocation( senderEndpoint, details );
    
    InstanceContainer* container = _instanceMan->getInstance( details.instanceID );
    
    CORAL_DLOG( INFO ) << "Received an invocation from " << senderEndpoint << " for instance " 
                             << details.instanceID;
    
    if( !container )
        CORAL_THROW( RemotingException, "No such instance of id: " << details.instanceID );
    
    co::IService* facet = container->getService( details.facetIdx );
    
    if( !facet )
        CORAL_THROW( RemotingException, "No such facet of index: " << details.facetIdx );
    
    co::IInterface* memberOwner = facet->getInterface();
    
    // if method is inherited
    if( details.typeDepth != -1 )
    {
        co::Range<co::IInterface* const> superTypes = memberOwner->getSuperTypes();
        
        if( details.typeDepth >= superTypes.getSize() || details.typeDepth < -1 )
            CORAL_THROW( RemotingException, "No such member owner of depth: " << details.typeDepth );
            
        memberOwner = superTypes[details.typeDepth];
    }
        
    co::IReflector* reflector = memberOwner->getReflector();
    
    co::Range<co::IMember* const> members = memberOwner->getMembers();
    
    if( details.memberIdx >= members.getSize() || details.memberIdx < 0 )
        CORAL_THROW( RemotingException, "No such member of index: " << details.memberIdx );
    
    co::IMember* member = members[details.memberIdx];
    co::MemberKind kind = member->getKind();
    
    // ------ Proceed to the actual invocation ------ //
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
   
    // Marshals the return from the call if applicable
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
    
    CORAL_DLOG( INFO ) << "Invoked method " << method->getName() << " of service "
    << facet->getInterface()->getName();
}
    
void Invoker::onGetField( co::IService* facet, co::IField* field, 
                             co::IReflector* refl, co::Any& returned )
{
    refl->getField( facet, field, returned );
    
    CORAL_DLOG( INFO ) << "Got field " << field->getName() << " of service "
    << facet->getInterface()->getName();
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
    
    CORAL_DLOG( INFO ) << "Set field " << field->getName() << " of service "
    << facet->getInterface()->getName();
}
    
void Invoker::getRefType( ReferenceType& refTypeInfo, co::Any& param, 
                             co::RefVector<co::IObject>& tempRefs )
{
    co::IObject* instance;
    switch( refTypeInfo.owner )
    {
        case OWNER_RECEIVER:
        {
            instance = _instanceMan->getInstance( refTypeInfo.instanceID )->getInstance();
            
            CORAL_DLOG( INFO ) << "A Parameter is a reference to local instance " << 
                    refTypeInfo.instanceID;
            break;
        }
        case OWNER_SENDER:
        case OWNER_ANOTHER:
        {
            Requestor* req = _requestorMan->getOrCreateRequestor( refTypeInfo.ownerEndpoint );
            instance = req->getOrCreateProxy( refTypeInfo.instanceID, refTypeInfo.instanceType );
            
            CORAL_DLOG( INFO ) << "A Parameter is a reference to a remote instance " << 
                    refTypeInfo.instanceID << " from : " << refTypeInfo.ownerEndpoint;
            
            break;
        }
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
            CORAL_DLOG( INFO ) << "The return value is a remote reference to a third node, requesting a lease now.";
            
            requestor->requestLease( refType.instanceID, senderEndpoint );
            refType.owner = OWNER_ANOTHER;
            refType.ownerEndpoint = requestor->getEndpoint();
        }
    }
}

} // namespace rpc


