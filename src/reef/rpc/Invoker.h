#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include "Unmarshaller.h"
#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/RefVector.h>
#include <co/IObject.h>

#include <string>

namespace reef {
namespace rpc {

class Node;
    
// Server-side implementation of IChannel. Delivers the appropriate calls to the Objects
class Invoker
{
public:
    Invoker( Node* node, co::IObject* object );
    
     ~Invoker();
    
    // Expects a unmarshaller in a before-decoding-a-call-msg state (see Unmarshaller).
    void onCallOrField( Unmarshaller& unmarshaller, co::Any* retValue = 0 );
    
    inline co::IComponent* getComponent() {   return _object->getComponent(); }
    
    inline co::IObject* getObject() { return _object.get(); }

       
private:

    // Called by onCall. Extract the parameters from unmarshaller and call method via reflector
    void onMethod( Unmarshaller& unmarshaller, co::int32 facetIdx, co::IMethod* method, 
                  co::Any* retValue = 0 );
    
    void onField( Unmarshaller& unmarshaller, co::int32 facetIdx, co::IField* field, co::Any* retValue = 0 );
    
    // TODO: remove the last param in coral 0.8
    void onGetParam( Unmarshaller& unmarshaller, co::IType* paramType, co::Any& param, 
                    co::RefVector<co::IObject>& tempRefs );
   
private:
    co::RefPtr<co::IObject> _object;

    co::int32 _remoteRefCount;
    
    co::IComponent* _component;
    
    Node* _node;
    
    // initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
    std::vector<co::IInterface*> _openedInterfaces;
	std::vector<co::IReflector*> _openedReflectors;    
};
    
}
    
} // namespace reef

#endif
