#include <gtest/gtest.h>

#include "FakeSocket.h"
#include "network/Connection.h"
#include "Channel.h"
#include <RemoteObject.h>
#include <Message.pb.h>

#include <testModule/ISimpleTypes.h>
#include <testModule/IComplexTypes.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

#include <iostream>

namespace reef
{
    
class FakeDelegate : public OutputChannelDelegate
{
public:
    
    // OutputChannelDelegate Methods
    int onNewInstance( Channel* channel, const std::string& typeName ) 
    { 
        _lastTypeName = typeName;
        return _instanceIdToReturn;
    }
    
    virtual void onSendCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) 
    {
        _lastServiceId = serviceId;
        _lastMethod = method;
        co::assign( args, _lastArguments );
    }
    virtual void onCall( Channel* channel, co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) 
    {
        _lastServiceId = serviceId;
        _lastMethod = method;
        co::assign( args, _lastArguments );
        result.set<co::int32>( _returnValue );
    }
    
    virtual void onGetField( Channel* channel, co::int32 serviceId, co::IField* field, co::Any& result ) 
    {
        _lastServiceId = serviceId;
        _lastField = field;
        result.set<co::int32>( _returnValue ); 
    }
    
    virtual void onSetField( Channel* channel, co::int32 serviceId, co::IField* field, const co::Any& value ) 
    {
        _lastServiceId = serviceId;
        _lastField = field;
        _lastValue = value;
    }
    
    void clearAll()
    {
        _lastTypeName = "";
        _lastServiceId = 0;
        _lastMethod = 0;
        _lastField = 0;
        _lastValue.set<co::int32>( -1 );
        _lastArguments.clear();
    }
    
public:
    co::int32 _instanceIdToReturn;
    co::int32 _returnValue;
    
    std::string _lastTypeName;
    co::int32 _lastServiceId;
    co::IMethod* _lastMethod;
    co::IField* _lastField;
    
    co::Any _lastValue;
    std::vector<co::Any> _lastArguments;

};
    
/* This class triggers the receive function of the Binder. This is necessary because
 if a message sent from a Connecter to the Binder would need return, then,
 the Connecter would be blocked waiting for a return, and we would not be able to
 call Binder's receive function that would post the return value for the Connecter. */
class BinderMsgTarget : public MsgTarget
{
public:
    // register the binder. Its receive function will be called whenever
    // a msg is sent to FakeSocket, and the msg will be passed to Channel
    BinderMsgTarget( Binder* binder, OutputChannel* outputChannel ) : 
    _outputChannel( outputChannel ), _binder( binder )
    {}
    
    void msgSent()
    {
        _binder->receive( _receivedMessage );
        Message msg;
        msg.ParseFromString( _receivedMessage );
        _outputChannel->write( &msg );
    }
    
    const std::string& getReceivedMessage()
    {
        return _receivedMessage;
    }
    
private:
    std::string _receivedMessage;
    OutputChannel* _outputChannel;
    Binder* _binder;
};

TEST( CompleteTests, CompleteSimpleParams )
{
    FakeDelegate fakeDelegate;
    fakeDelegate._instanceIdToReturn = 5;
    
    Binder* b1 = new Binder();
    b1->bind( "addr1" );
    Connecter* c1 = new Connecter();
    c1->connect( "addr1" );
    
    
    co::IObject* dummyObj = co::newInstance( "testModule.TestComponent" );
    OutputChannel* oc = new OutputChannel( dummyObj, b1 ); // use the same object for the server!
    oc->setDelegate( static_cast<OutputChannelDelegate*>( &fakeDelegate ) );
    oc->setId( 2 );
    BinderMsgTarget msgTarget( b1, oc );
    FakeSocket::setMsgTarget( static_cast<MsgTarget*>( &msgTarget ), "addr1" );
    
    InputChannel* ic = new InputChannel( c1 );
    ic->setId( -1 );
    co::IComponent* componentType = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    co::IObject* TCObj = new RemoteObject( componentType, ic );
    
    EXPECT_EQ( fakeDelegate._lastTypeName, std::string( "testModule.TestComponent" ) );
    ic->setId( -1 );
    EXPECT_EQ( ic->newInstance( "test1" ), 5 );
    
}
    
}