#ifndef _REEF_ENCODER_H_
#define _REEF_ENCODER_H_

#include "Channel.h"

#include <co/Any.h>
#include <co/Range.h>

namespace reef
{
    
class Connecter;
class Message;
    
/*
    Client-side Channel. Receives the calls from the RemoteObject, encodes and sends to Server
 */
class Encoder : public Channel
{
public:
    Encoder( Connecter* connecter );
    ~Encoder();
    
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
private:
    // Writes an event into this input channel. The given event will be serialized over network.
    void write( const Message* message );
    
	void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );

private:
	Connecter* _connecter;
    
    co::int32 _decoderAddress;
};

}

#endif