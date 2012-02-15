#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include <co/RefPtr.h>
#include <co/IObject.h>

#include "Channel.h"

#include <string>

namespace reef
{

class ServerNode;
    
// Manages access to
class Servant : public OutputChannelDelegate
{
public:
    Servant( co::IObject* object );
    
    void onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void onGetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, co::Any& result );
    void onSetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value );
    
private:
    co::RefPtr<co::IObject> _object;

	struct QuickInvokeParam
	{
		co::IService* serv;
		co::IReflector* refl;

		QuickInvokeParam( co::IService* service, co::IReflector* reflector );
	};

	std::vector<QuickInvokeParam> _openedServices;
};

} // namespace reef

#endif
