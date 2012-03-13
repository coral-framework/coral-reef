#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include "IChannel.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#include <string>

namespace reef
{
    
class ServerNode;
    
// Server-side implementation of IChannel. Delivers the appropriate calls to the Objects
class Servant : public IChannel
{
public:
    Servant( co::IObject* object );
    
    // In case this Servant is intended for delivering newInstanceMessages
    void setServerNode( ServerNode* serverNode ) { _serverNode = serverNode; }
    
    // IChannel methods
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
    co::IComponent* getComponent()
    {
        return _component;
    }
    
private:
    co::RefPtr<co::IObject> _object;
    co::IComponent* _component;
    
    ServerNode* _serverNode;

	// initializes _openedService's and Reflector's index for the accessed service
	void onServiceFirstAccess( co::int32 serviceId );
	std::vector<co::IService*> _openedServices;
	std::vector<co::IReflector*> _openedReflectors;
};

} // namespace reef

#endif
