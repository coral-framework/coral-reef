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
    Channel( Connection* connection );
    virtual ~Channel();
    
    void setConnection( Connection* connection );
    int getId() { return _channelId; }
    
    /*
        Establishes a new communication channel to a new remote instance specified by 
        \a remoteTypeName. A new instance of \a remoteTypeName will be created at 
        server side and bounded to this channel. The unique id of the remote instance 
        will be retrieved.
     
        This method makes this channel permanently bound to the new remote instance
        and retrieves -1 if it had already been established.
     */
    virtual int establish( const std::string& remoteTypeName ) = 0;
    
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) = 0;
    virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) = 0;
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result ) = 0;
    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value ) = 0;
    
    // Template method:
    // Handles the message and translate it into an event actual (sendCall, call..)
    // Returns false if the message destination is not this channel or the message is malformed.
    bool handleMessage( const std::string& message );
    
public:
    // Checks whether the given string is a well formed message
    static bool isMessageValid( const std::string& message );
    
protected:
    // channleId is same as remote instance id
    int _channelId;
    Connection* _connection;
    std::stringstream _stream;
};
    
class InputChannel : public Channel
{
public:
    InputChannel( Connection* connection );
    ~InputChannel();
    
    int establish( const std::string& remoteTypeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );
};

class OutputChannelDelegate
{
public:
    virtual void onChannelEstablished( Channel* channel, const std::string& typeName );
    virtual void onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) = 0;
    virtual void onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) = 0;
    virtual void onGetField( Channel* channel, co::int32 serviceId, co::IField* field, co::Any& result ) = 0;
    virtual void onSetField( Channel* channel, co::int32 serviceId, co::IField* field, const co::Any& value ) = 0;
};
    
class OutputChannel : public Channel
{
public:
    OutputChannel( Connection* connection, OutputChannelDelegate* delegate );
    ~OutputChannel();
    
    int establish( const std::string& remoteTypeName ) = 0;
    
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) = 0;
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) = 0;
    void getField( co::int32 serviceId, co::IField* field, co::Any& result ) = 0;
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value ) = 0;
    
private:
    OutputChannelDelegate* _delegate;
};
    
} // namespace reef

#endif
