#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include "Marshaller.h"
#include "Unmarshaller.h"
#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/RefVector.h>
#include <co/IObject.h>

#include <string>

namespace reef {
namespace rpc {

class Node;
    
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
    Invoker( Node* node, co::IObject* object );
    
     ~Invoker();
    
    /*!
        Makes an invocation in the actual object based on the to-be-unmarshalled data inside the
        parameter \unmarshaller (the unmarshalling methods will be called)
        \param unmarshaller An unmarshaller with a call request already set ofr unmarshalling.
        \param isSynch True if it is a synchronous call, and a return is expected
        \param returned What the invocation returned. Already marshalled and ready to send.     
    */
    void invoke( Unmarshaller& unmarshaller, bool isSynch, std::string& returned );
    
    //! returns the object controlled by this Invoker
    inline co::IObject* getObject() { return _object.get(); }
       
private:

    void onMethod( Unmarshaller& unmarshaller, co::IService* facet, co::IMethod* method, 
                  co::IReflector* refl, co::Any& returned );
    
    void onGetField( Unmarshaller& unmarshaller, co::IService* facet, co::IField* field, 
                    co::IReflector* refl, co::Any& returned );
    
    void onSetField( Unmarshaller& unmarshaller, co::IService* facet, co::IField* field, 
                    co::IReflector* refl );
    
    // TODO: remove the last param in coral 0.8
    void unmarshalParameter( Unmarshaller& unmarshaller, co::IType* paramType, co::Any& param, 
                    co::RefVector<co::IObject>& tempRefs );
    
    // Identify and marshals an interface that has been returned from an invoke
    void onInterfaceReturned( co::IService* returned, std::string& caller, 
                             std::string& marshalledReturn );
   
private:
    co::RefPtr<co::IObject> _object;

    co::int32 _remoteRefCount;
    
    co::IComponent* _component;
    
    Node* _node;
    
    Marshaller _marshaller;
    
    // initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
    std::vector<co::IInterface*> _openedInterfaces;
	std::vector<co::IReflector*> _openedReflectors;    
};
    
}
    
} // namespace reef

#endif
