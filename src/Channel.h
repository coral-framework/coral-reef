#ifndef _REEF_CHANNEL_H_
#define _REEF_CHANNEL_H_

#include <co/Any.h>
#include <co/Coral.h>

#include "network/Connection.h"

#include <sstream>



namespace reef 
{

class Event;

//! Template abstract class.
class Channel
{
public:
    // Routes the given message to the proper channel using message destination identifier.
    static void route( const std::string& data, const std::vector<Channel*>& channels );
    
public:
    Channel( Connection* connection );
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
    virtual void write( const reef::Event* event ) = 0;
    
protected:
    int _channelId;
    Connection* _connection;
};

// A channel that converts events into raw messages over network connections
class InputChannel : public Channel
{
public:
    InputChannel( Connection* connection );
    ~InputChannel();
    
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result );
    void setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value );
    
    // Writes an event into this input channel. The given event will be serialized over network.
    void write( const reef::Event* event );
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
    OutputChannel( Connection* connection, OutputChannelDelegate* delegate );
    ~OutputChannel();
    
    int newInstance( const std::string& typeName );
    
    void sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result );
    void setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value );
    
    // Writes an event into an output channel. It will translate the event into a call
    // of one of the above methods (sendCall, call, getField, setField... )
    void write( const reef::Event* event );
    
private:
    OutputChannelDelegate* _delegate;
};
    
} // namespace reef

#endif
