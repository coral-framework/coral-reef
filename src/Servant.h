#ifndef _REEF_SERVANT_H_
#define _REEF_SERVANT_H_

#include <co/RefPtr.h>
#include <co/IObject.h>

#include "Channel.h"

#include <string>

namespace reef
{
    
// Manages access to
class Servant : public OutputChannelDelegate
{
public:
    Servant( const std::string& type );
    
    void onSendCall( Channel* channel, co::int32 facetId, co::int32 methodId, co::Range<co::Any const> args );
    void onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void onGetField( Channel* channel, co::int32 serviceId, co::IField* field, co::Any& result );
    void onSetField( Channel* channel, co::int32 serviceId, co::IField* field, const co::Any& value );
    
private:
    co::RefPtr<co::IObject> _object;
};

} // namespace reef

#endif