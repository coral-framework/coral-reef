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
#include <ctime>

namespace rpc {

    
Invoker::Invoker( InstanceManager* instanceMan, BarrierManager* barrierMan, ServerRequestHandler* srh, 
            RequestorManager* requestorMan ) : _instanceMan( instanceMan ), _barrierMan( barrierMan ),
                _srh( srh ), _requestorMan( requestorMan ), _barrierUp( false )
{
}
    
Invoker::~Invoker()
{
}

void Invoker::dispatchInvocation( const std::string& inputStream )
{    
    std::string returnValue;
    
    try
    {
        MessageType msgType = _demarshaller.demarshal( inputStream );
        
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

void Invoker::hitBarrier( co::uint32 timeout )
{
	time_t tstart = time(0);
	time_t current;
	current = time(0);

    if( current - tstart > timeout )
	{
		CORAL_THROW( RemotingException, "Reply receiving timeout" );
	}

    while( !_barrierUp )
	{ // tried to enter the barrier before its raising
		current = time(0);
		_srh->react();

		if( current - tstart > timeout )
		{
			CORAL_THROW( RemotingException, "Reply receiving timeout" );
		}
	}
    
    _barrierCreator->requestBarrierHit();
    
    while( _barrierUp )
	{
		current = time(0);
		_srh->react();

		if( current - tstart > timeout )
		{
			CORAL_THROW( RemotingException, "Reply receiving timeout" );
		}
	}
}
    
void Invoker::invokeManagement( Demarshaller& demarshaller, MessageType msgType, 
                            std::string& outputStream )
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
            
            //TODO implement the leasing system.
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
    
    _marshaller.marshalIntReturn( returnID , outputStream );
}
    
void Invoker::invokeInstance( Demarshaller& demarshaller, std::string& outputStream )
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
        co::TSlice<co::IInterface*> superTypes = memberOwner->getSuperTypes();
        
        if( details.typeDepth >= superTypes.getSize() || details.typeDepth < -1 )
            CORAL_THROW( RemotingException, "No such member owner of depth: " << details.typeDepth );
            
        memberOwner = superTypes[details.typeDepth];
    }
        
    co::IReflector* reflector = memberOwner->getReflector();
    
    co::TSlice<co::IMember*> members = memberOwner->getMembers();
    
    if( details.memberIdx >= members.getSize() || details.memberIdx < 0 )
        CORAL_THROW( RemotingException, "No such member of index: " << details.memberIdx );
    
    co::IMember* member = members[details.memberIdx];
    co::MemberKind kind = member->getKind();
    
    if( kind == co::MK_METHOD )
    {
        co::IMethod* method = co::cast<co::IMethod>( member );
        onMethod( puller, facet, method, reflector, senderEndpoint, outputStream );
        return;
    }   
    else   
    {
        co::IField* field = co::cast<co::IField>( member );
        if( !details.hasReturn )
        {
            onSetField( puller, facet, field, reflector );
            return;
        }
        else
        {
            onGetField( facet, field, reflector, outputStream );
        }
    }
}

void Invoker::onMethod( ParameterPuller& puller, co::IService* facet, co::IMethod* method, 
                           co::IReflector* refl, const std::string& senderEndpoint, std::string& outputStream )
{    
    std::vector<co::AnyValue> anyValues;
    std::vector<co::Any> args;
    co::TSlice<co::IParameter*> params = method->getParameters(); 
    
    size_t size = params.getSize();
    args.resize( size );
    anyValues.resize( size );
    
    // Set the anys to point to the anyvalues
    for( int i = 0; i < size; i++ )
        args[i].set( anyValues[i] );
    
    for( int i = 0; i < size; i++ )
    {
        co::IType* paramType = params[i]->getType();
		anyValues[i].create( paramType );
        if( paramType->getKind() != co::TK_INTERFACE )
        {
            if( params[i]->getIsIn() )
				puller.pullValue( paramType, anyValues[i].getAny() );
        }
        else
        {
            ReferenceType refTypeInfo;
			if( params[i]->getIsIn() )
			{
				puller.pullReference( refTypeInfo );
				getRefType( refTypeInfo, anyValues[i] );
			}
        }
    }

	// ------ Proceed to the actual invocation ------ //
    co::AnyValue returnValue;
	bool hasOutput = false;
    refl->invoke( facet, method, args, returnValue );
    
    CORAL_DLOG( INFO ) << "Invoked method " << method->getName() << " of service "
    << facet->getInterface()->getName();

	ParameterPusher& pusher = _marshaller.beginOutput();

	co::IType* returnType = method->getReturnType();
	if( returnType )
	{
		hasOutput = true;
		if( returnType->getKind() == co::TK_INTERFACE )
			CORAL_THROW( RemotingException, "References can't be passed via return, must use out params" );

		pusher.pushValue( returnValue.getAny().asIn(), returnType );
	}

	for( int i  = 0; i < params.getSize(); i++ )
	{
		if( params[i]->getIsOut() )
		{
			co::IType* paramType = params[i]->getType();
			if( paramType->getKind() != co::TK_INTERFACE )
			{
				pusher.pushValue( args[i].asIn(), paramType );
			}
			else
			{
				ReferenceType refType;
				getRefTypeInfo( args[i].get<co::IService*>(), senderEndpoint, refType );
				pusher.pushReference( refType );
			}
			hasOutput = true;
		}
	}

	// needed even if there is no return in order to reset the marshaller internal state
	_marshaller.marshalOutput( outputStream );
	
	if( !hasOutput )
		outputStream = "";
}

void Invoker::onGetField( co::IService* facet, co::IField* field, 
                             co::IReflector* refl, std::string& outputStream )
{
	co::AnyValue returnValue;
    refl->getField( facet, field, returnValue );
    
	co::IType* fieldType = field->getType();
	if( fieldType->getKind() == co::TK_INTERFACE )
			CORAL_THROW( RemotingException, "References can't be passed via return, must use out params" );

	ParameterPusher& pusher = _marshaller.beginOutput();
	pusher.pushValue( returnValue.getAny().asIn(), fieldType );

	_marshaller.marshalOutput( outputStream );

    CORAL_DLOG( INFO ) << "Got field " << field->getName() << " of service "
    << facet->getInterface()->getName();
}

void Invoker::handleOutput( ParameterPusher& pusher, const co::Any& output, 
					 co::IType* outputType, const std::string& senderEndpoint )
{
	if( outputType->getKind() != co::TK_INTERFACE )
	{
		pusher.pushValue( output, outputType );
	}
	else
	{
		ReferenceType refType;
		getRefTypeInfo( output.get<co::IService*>(), senderEndpoint, refType );
		pusher.pushReference( refType );
	}
}

void Invoker::onSetField( ParameterPuller& puller, co::IService* facet, co::IField* field, 
                             co::IReflector* refl )
{
    co::AnyValue av; co::Any arg( av );
    
    co::IType* fieldType = field->getType();
    
    if( fieldType->getKind() != co::TK_INTERFACE )
    {
        av.create( fieldType );
        puller.pullValue( fieldType, av.getAny() );
    }
    else
    {
        ReferenceType refTypeInfo;
        puller.pullReference( refTypeInfo );
        getRefType( refTypeInfo, av );
    }
    
    refl->setField( facet, field, arg );
    
    CORAL_DLOG( INFO ) << "Set field " << field->getName() << " of service "
    << facet->getInterface()->getName();
}
    
void Invoker::getRefType( ReferenceType& refTypeInfo, const co::Any& ret )
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
    
    co::TSlice<co::IPort*> ports = instance->getComponent()->getFacets();
    
    co::IPort* port = ports[refTypeInfo.facetIdx];
    co::IService* service = instance->getServiceAt( port );
    ret.put( service );
}

void Invoker::getRefTypeInfo( co::IService* service, const std::string& senderEndpoint, 
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


