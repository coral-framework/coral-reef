#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include "Decoder.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#include <string>

namespace reef
{
    
// Server-side implementation of IChannel. Delivers the appropriate calls to the Objects
class Servant
{
public:
    Servant( co::IObject* object );
    
     ~Servant();
    
    // Expects a decoder in a before-decoding-a-call-msg state (see Decoder).
    void onCallOrField( Decoder& decoder, co::Any* retValue = 0 );
    
    inline co::IComponent* getComponent() {   return _object->getComponent(); }
    
    inline co::IObject* getObject() { return _object.get(); }
protected:
    co::RefPtr<co::IObject> _object;
   
private:

    // Called by onCall. Extract the parameters from decoder and call method via reflector
    void onMethod( Decoder& decoder, co::int32 facetIdx, co::IMethod* method, 
                  co::Any* retValue = 0 );
    
    void onField( Decoder& decoder, co::int32 facetIdx, co::IField* field, co::Any* retValue = 0 );
    
    void onGetParam( Decoder& decoder, co::IType* paramType, co::Any& param );
    
    co::int32 _remoteRefCount;
    
    // initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
    std::vector<co::IInterface*> _openedInterfaces;
	std::vector<co::IReflector*> _openedReflectors;
};

} // namespace reef

#endif
