#ifndef _REEF_ENCODER_CHANNEL_H_
#define _REEF_ENCODER_CHANNEL_H_

#include <co/Any.h>
#include <co/Coral.h>

#include "network/Connection.h"

#include <sstream>

namespace reef 
{

class Message;
class Message_New;
class Message_Member;
class Servant;
class ServerNode;

// A channel that converts raw message writes into events that can be delegated
class Decoder
{
public:
    // Routes the given message to the proper channel using message destination identifier.
    void routeAndDeliver( const std::string& data, const std::vector<Servant*>& channels );
    // uses the internal Servant vector to route
    void routeAndDeliver( const std::string& data );
    
    void deliver( Message* msg, Servant* destination );
    
    
    Decoder( Binder* binder );
    ~Decoder();
    
private:
    void deliverNew( const Message_New* subMessage );
    void deliverCall( const Message_Member* subMessage );
    void deliverField( const Message_Member* subMessage );
    
    
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
private:
    
	Binder* _binder;

    Servant* _destination;
    
    std::vector<Servant*> _channels;
};
    
} // namespace reef

#endif
