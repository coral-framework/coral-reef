#ifndef _REEF_INVOKER_H_
#define _REEF_INVOKER_H_

#include "Marshaller.h"
#include "Demarshaller.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/RefVector.h>
#include <co/IObject.h>

#include <string>

namespace reef {
namespace rpc {

class Node;
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
    Invoker( Node* node, ServerRequestHandler* srh, RequestorManager* requestorMan );
    
     ~Invoker();
    
    void dispatchInvocation( const std::string& invocation );
    
    /*!
        Makes an invocation in the actual object based on the to-be-demarshalled data inside the
        parameter \demarshaller (the demarshalling methods will be called)
        \param demarshaller An demarshaller with a call request already set ofr demarshalling.
        \param isSynch True if it is a synchronous call, and a return is expected
        \param returned What the invocation returned. Already marshalled and ready to send.     
    */
    void invokeInstance( Demarshaller& demarshaller, co::int32 instanceID, bool isSynch, 
                        std::string& returned );
    
    void invokeManager( Demarshaller& demarshaller, Demarshaller::MsgType type, bool isSynch, 
                       std::string& returned );
       
private:

    // TODO REMOTINGERROR when mismatching parameter types/return facetIds ...
    void onMethod( Demarshaller& demarshaller, co::IService* facet, co::IMethod* method, 
                  co::IReflector* refl, co::Any& returned );
    
    void onGetField( Demarshaller& demarshaller, co::IService* facet, co::IField* field, 
                    co::IReflector* refl, co::Any& returned );
    
    void onSetField( Demarshaller& demarshaller, co::IService* facet, co::IField* field, 
                    co::IReflector* refl );
    
    // TODO: remove the last param in coral 0.8
    void demarshalParameter( Demarshaller& demarshaller, co::IType* paramType, co::Any& param, 
                    co::RefVector<co::IObject>& tempRefs );
    
    // Identify and marshals an interface that has been returned from an invoke
    void onInterfaceReturned( co::IService* returned, std::string& caller, 
                             std::string& marshalledReturn );

private:
    
    Node* _node;
    InstanceManager* _instanceMan;
    ServerRequestHandler* _srh;
    
    RequestorManager* _requestorMan;
    
    Marshaller _marshaller;
    Demarshaller _demarshaller;
};
    
}
    
} // namespace reef

#endif
