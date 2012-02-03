#ifndef _REEF_CHANNEL_H_
#define _REEF_CHANNEL_H_

#include <co/Any.h>
#include <co/Coral.h>

#include "network/Connection.h"

#include <sstream>

namespace reef 
{

//! Template abstract class.
class Channel
{
public:
    // Routes the given message to the proper channel using message destination identifier.
    static void route( std::string& msg, const std::vector<Channel*>& channels );
    
public:
    Channel();
    virtual ~Channel();
    
    int getId() { return _channelId; }
       
    // Creates a new instance and retrieves its unique id.
    virtual void newInstance( const std::string& typeName ) = 0;
    
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) = 0;
    virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) = 0;
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result ) = 0;
    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value ) = 0;

    // Writes a raw message into channel.
    virtual void write( const std::string& rawMessage ) = 0;
    
protected:
    int _channelId;
};

// A channel that converts events into raw messages
class InputChannel : public Channel
{
public:
    InputChannel( Connection* connection );
    ~InputChannel();
    
    void newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
    void write( const std::string& rawMessage );
    
protected:
    Connection* _connection;
    std::stringstream _sstream;
};

class OutputChannelDelegate
{
public:
    virtual void onNewInstance( Channel* channel, const std::string& typeName ) {;}
    virtual void onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) {;}
    virtual void onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) {;}
    virtual void onGetField( Channel* channel, co::int32 serviceId, co::IField* field, co::Any& result ) {;}
    virtual void onSetField( Channel* channel, co::int32 serviceId, co::IField* field, const co::Any& value ) {;}
};

// A channel that converts raw message writes into events that can be delegated
class OutputChannel : public Channel
{
public:
    OutputChannel( OutputChannelDelegate* delegate );
    ~OutputChannel();
    
    void newInstance( const std::string& typeName );
    
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
    
    void write( const std::string& rawMessage );
    
private:
    OutputChannelDelegate* _delegate;
};
    
} // namespace reef

#endif
