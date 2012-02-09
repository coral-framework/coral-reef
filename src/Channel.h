#ifndef _REEF_CHANNEL_H_
#define _REEF_CHANNEL_H_

#include <co/Any.h>
#include <co/Coral.h>

#include "network/Connection.h"

#include <sstream>

namespace reef 
{

class Message;

//! Template abstract class.
class Channel
{
public:
    // Routes the given message to the proper channel using message destination identifier.
    static void route( const std::string& data, const std::vector<Channel*>& channels );
    
public:
    Channel();
    virtual ~Channel();
    
    void setId( int id ) { _channelId = id; }
    int getId() { return _channelId; }
       
    // Creates a new instance and retrieves its unique id.
    virtual int newInstance( const std::string& typeName ) = 0;
    
    virtual void sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args ) = 0;
    virtual void call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result ) = 0;
    virtual void getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result ) = 0;
    virtual void setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value ) = 0;

    // Writes a raw event into channel.
    virtual void write( const Message* message ) = 0;
    
protected:
    int _channelId;
};

// A channel that converts events into raw messages over network connections
class InputChannel : public Channel
{
public:
    InputChannel( Connecter* connecter );
    ~InputChannel();
    
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result );
    void setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value );
    
protected:
    // Writes an event into this input channel. The given event will be serialized over network.
    void write( const Message* message );

protected:
	Connecter* _connecter;
};

class OutputChannelDelegate
{
public:
    virtual int onNewInstance( Channel* channel, const std::string& typeName ) { return -1; }
    virtual void onSendCall( Channel* channel, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args ) {;}
    virtual void onCall( Channel* channel, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result ) {;}
    virtual void onGetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, co::Any& result ) {;}
    virtual void onSetField( Channel* channel, co::int32 serviceId, co::int32 fieldIndex, const co::Any& value ) {;}
};

// A channel that converts raw message writes into events that can be delegated
class OutputChannel : public Channel
{
public:
    OutputChannel( Binder* binder, OutputChannelDelegate* delegate );
    ~OutputChannel();
    
    int newInstance( const std::string& typeName );
    
    void sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result );
    void setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value );
    
    // Writes a message into an output channel. This channel will translate the message into a call
    // of one of the above methods (sendCall, call, getField, setField... )
    void write( const Message* message );
    
private:
	Binder* _binder;
    OutputChannelDelegate* _delegate;
};
    
} // namespace reef

#endif
