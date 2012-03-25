#ifndef _REEF_ENCODER_H_
#define _REEF_ENCODER_H_

#include <co/Any.h>
#include <co/Range.h>

namespace reef
{
    
class Connecter;
class Message;
    
/*
    Client-side Channel. Receives the calls from the RemoteObject, encodes and sends to Server
 */
class Encoder
{
public:
    Encoder( Connecter* connecter );
    virtual ~Encoder();
    
    virtual int newInstance( const std::string& typeName );
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, 
                          co::Range<co::Any const> args );
    virtual void call( co::int32 serviceId, co::IMethod* method, 
                      co::Range<co::Any const> args, co::Any& result );
    virtual void getField( co::int32 serviceId, co::IField* field, 
                          co::Any& result );
    virtual void setField( co::int32 serviceId, co::IField* field, 
                          const co::Any& value );
    
private:
    // Writes an event into this input channel. The given event will be serialized over network.
    void write( const Message* message );
    
	void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );

private:
    co::RefPtr<Connecter> _connecter;
    
    co::int32 _decoderAddress;
};

}

#endif