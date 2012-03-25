#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#include <string>

namespace reef
{
    
class ServerNode;
    
// Server-side implementation of IChannel. Delivers the appropriate calls to the Objects
class Servant
{
public:
    Servant( co::IObject* object );
    
    virtual ~Servant();
    
    // In case this Servant is intended for delivering newInstanceMessages
    virtual void setServerNode( ServerNode* serverNode ) { _serverNode = serverNode; }
    
    // IChannel methods
    virtual int newInstance( const std::string& typeName );
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
    inline co::IComponent* getComponent()
        {   return _object->getComponent(); }
protected:
    co::RefPtr<co::IObject> _object;
   
private:
    ServerNode* _serverNode;

	// initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
	std::vector<co::IReflector*> _openedReflectors;
};

} // namespace reef

#endif
