#ifndef _RPC_INVOKER_H_
#define _RPC_INVOKER_H_

#include "Marshaller.h"
#include "Demarshaller.h"
#include "Requestor.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#include <string>

namespace rpc {

class Node;
class BarrierManager;
class InstanceManager;
class RequestorManager;
class InstanceContainer;
class ServerRequestHandler;
    
//! Delivers the appropriate calls to an Object
class Invoker
{
public:
    /*!
        Constructor
        \param node The node that is creating the Invoker. The invoker needs the node to request 
        proxys for reference parameters
        \param object The object that will be controlled by this Invoker
    */
    Invoker( InstanceManager* instanceMan, BarrierManager* barrierMan, ServerRequestHandler* srh, 
            RequestorManager* requestorMan );
    
     ~Invoker();
    
    void dispatchInvocation( const std::string& inputStream );
    
    // Waits for other node to raise a barrier if it is still not risen, blocks until barrier is down
    void hitBarrier( co::uint32 timeout );
    
private:
    
    void invokeManagement( Demarshaller& demarshaller, MessageType msgType,
                                 std::string& outputStream );
    
    /*!
     Makes an invocation in the actual object based on the to-be-demarshalled data inside the
     parameter \demarshaller (the demarshalling methods will be called)
     \param demarshaller An demarshaller with a call request already set ofr demarshalling.
     \param returned What the invocation returned. Already marshalled and ready to send.     
     */
    void invokeInstance( ParameterPuller& puller, const std::string& senderEndpoint, const InvocationDetails& details, std::string& outputStream );
    
    void onMethod( ParameterPuller& puller, co::IService* facet, co::IMethod* method, 
                  co::IReflector* refl, const std::string& senderEndpoint, std::string& outputStream );
    
    void onGetField( co::IService* facet, co::IField* field, 
                    co::IReflector* refl, std::string& outputStream );
    
    void onSetField( ParameterPuller& puller, co::IService* facet, co::IField* field, 
                    co::IReflector* refl );
    
	// receives an Any with something that an invoke has output. Handles the value and pushes it.
	void handleOutput( ParameterPusher& pusher, const co::Any& output, 
					 co::IType* outputType, const std::string& senderEndpoint );

    // Whenever a ref type parameter arrives from a invocation, this method decodes it into an 
    // actual instance. This function may call another node and possibly block.
    void getRefType( ReferenceType& refTypeInfo, const co::Any& ret );
    
    // Identify an interface that has been returned from an invoke and return its details for marshalling
    void getRefTypeInfo( co::IService* service, const std::string& senderEndpoint, 
                             ReferenceType& refType );

private:
    
    InstanceManager* _instanceMan;
    BarrierManager* _barrierMan;
    ServerRequestHandler* _srh;
    
    RequestorManager* _requestorMan;
    
    Marshaller _marshaller;
    Demarshaller _demarshaller;
    
    bool _barrierUp;
    co::RefPtr<Requestor> _barrierCreator;
};
    
} // namespace rpc

#endif
