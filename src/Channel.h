#ifndef _REEF_Channel_H_
#define _REEF_Channel_H_

#include <co/Any.h>
#include <co/Range.h>
#include <co/Coral.h>

namespace reef {
    
/*
 This interface promotes an high level API for all the RPCs necessary. Any component that shall
 use RPCs for communicating with remote components will do so through this API. There will be
 at least a client-side and a server-side implementation of Channel.
*/
class Channel
{
public:
    virtual int newInstance( const std::string& typeName ) = 0;
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) = 0;
    virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) = 0;
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result ) = 0;
    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value ) = 0;
    
    // The component that this Channel works for (owner of the methods and fields)
    co::IComponent* getComponent(){ return _component; }
    void setComponent( co::IComponent* component ){ _component = component; }
    
protected:
    
    co::IComponent* _component;
};
    

}

#endif