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
    void onCall( Decoder& decoder, co::Any& retValue );
    
    // IChannel methods
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
    inline co::IComponent* getComponent() {   return _object->getComponent(); }
    
    inline co::IObject* getObject() { return _object.get(); }
protected:
    co::RefPtr<co::IObject> _object;
   
private:

    co::int32 _remoteRefCount;
    
    // initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
    std::vector<co::IInterface*> _openedInterfaces;
	std::vector<co::IReflector*> _openedReflectors;
};

} // namespace reef

#endif
