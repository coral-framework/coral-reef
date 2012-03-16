#include <gtest/gtest.h>

#include "FakeSocket.h"
#include "network/Connection.h"
#include "Channel.h"
#include "Encoder.h"
#include "Decoder.h"
#include <RemoteObject.h>
#include <Message.pb.h>
#include "Servant.h"

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
    
class FakeServant : public Channel
{
public:
    FakeServant() : _instanceIdToReturn( 1 )
    {
    }
    
    // DecoderChannel Methods
    int newInstance( const std::string& typeName ) 
    { 
        _lastTypeName = typeName;
        return _instanceIdToReturn;
    }
    
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args ) 
    {
        _lastServiceId = serviceId;
        _lastMethod = method;
        co::assign( args, _lastArguments );
    }
    void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result ) 
    {
        _lastServiceId = serviceId;
        _lastMethod = method;
        co::assign( args, _lastArguments );
        result.set<co::int32>( _returnValue );
    }
    
    void getField( co::int32 serviceId, co::IField* field, co::Any& result ) 
    {
        _lastServiceId = serviceId;
        _lastField = field;
        result.set<co::int32>( _returnValue ); 
    }
    
    void setField( co::int32 serviceId, co::IField* field, const co::Any& value ) 
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
    BinderMsgTarget( Binder* binder, Channel* channel ) : 
        _binder( binder ), _decoder( _binder ), _channel( channel )
    { 
    }
    
    void msgSent()
    {
        _binder->receive( _receivedMessage );
        _decoder.deliver( _receivedMessage, _channel );
    }
    
    const std::string& getReceivedMessage()
    {
        return _receivedMessage;
    }
    
private:
    std::string _receivedMessage;
    Binder* _binder;
    Decoder _decoder;
    Channel* _channel;
};

TEST( CompleteTests, ChannelToChannelTest )
{
    FakeServant fd;
    fd._instanceIdToReturn = 5;
    
    Binder b1;
    b1.bind( "addr1" );
    Connecter* c1 = Connecter::getOrOpenConnection( "addr1" );
    
    co::IComponent* dummyComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    co::IPort* dummySmplPort = co::cast<co::IPort>( dummyComponent->getMember( "simple" ) );
    co::RefPtr<co::IObject> dummyObj = co::newInstance( "testModule.TestComponent" );
    co::IInterface* dummySmplItf = dummyObj->getService<testModule::ISimpleTypes>()->getInterface();
    
    fd.setComponent( dummyComponent );
    BinderMsgTarget msgTarget( &b1, &fd );
    FakeSocket::setMsgTarget( static_cast<MsgTarget*>( &msgTarget ), "addr1" );
    
    Encoder* ic = new Encoder( c1 );
    
    co::RefPtr<co::IObject> dummyRemoteObj = new RemoteObject( dummyComponent, ic );
    testModule::ISimpleTypes* st = dummyRemoteObj->getService<testModule::ISimpleTypes>();
    
    EXPECT_EQ( ic->newInstance( "test1" ), 5 );
    EXPECT_EQ( fd._lastTypeName, std::string( "test1" ) );
    
    st->setInt( 99 );
    EXPECT_EQ( fd._lastServiceId, dummySmplPort->getIndex() );
    EXPECT_EQ( fd._lastMethod, co::cast<co::IMethod>( dummySmplItf->getMember( "setInt" ) ) );
    EXPECT_EQ( fd._lastArguments[0].get<co::int32>(), 99 );
    
    std::vector<co::int32> arg;
    fd.clearAll();
    st->setIntList( arg );
    EXPECT_EQ( fd._lastServiceId, dummySmplPort->getIndex() );
    EXPECT_EQ( fd._lastMethod, co::cast<co::IMethod>( dummySmplItf->getMember( "setIntList" ) ) );
    
    fd.clearAll();
    st->setDouble( 0.1 );
    EXPECT_EQ( fd._lastServiceId, dummySmplPort->getIndex() );
    EXPECT_EQ( fd._lastMethod, co::cast<co::IMethod>( dummySmplItf->getMember( "setDouble" ) ) );
    EXPECT_EQ( fd._lastArguments[0].get<double>(), 0.1 );
    
    fd.clearAll();
    st->setStoredInt( 12 );
    EXPECT_EQ( fd._lastServiceId, dummySmplPort->getIndex() );
    EXPECT_EQ( fd._lastField, co::cast<co::IField>( dummySmplItf->getMember( "storedInt" ) ) );
    EXPECT_EQ( fd._lastValue.get<co::int32>(), 12 );
   
    fd._returnValue = 12;
    EXPECT_EQ( st->getStoredInt(), 12 );
    
    fd._returnValue = 2;
    EXPECT_EQ( st->incrementInt( 1 ), 2 );
}
    
TEST( CompleteTests, ProxyToServantTest )
{
    Binder b1;
    b1.bind( "addr1" );
    Connecter* c1 = Connecter::getOrOpenConnection( "addr1" );
    
    co::IComponent* dummyComponent = co::cast<co::IComponent>( co::getType( "testModule.TestComponent" ) );
    co::RefPtr<co::IObject> dummyObj = co::newInstance( "testModule.TestComponent" );
    
    // now that newInstance was called, set the appropriate servant
    FakeServant fakeServant;
    
    BinderMsgTarget fakeMsgTarget( &b1, &fakeServant );
    FakeSocket::setMsgTarget( static_cast<MsgTarget*>( &fakeMsgTarget ), "addr1" );
    
    Encoder* ic = new Encoder( c1 );

    // newInstance will be called
    co::RefPtr<co::IObject> dummyRemoteObj = new RemoteObject( dummyComponent, ic );
    testModule::ISimpleTypes* st = dummyRemoteObj->getService<testModule::ISimpleTypes>();
    
    Servant servant( dummyObj.get() );
    BinderMsgTarget msgtarget( &b1, &servant );
    FakeSocket::setMsgTarget( static_cast<MsgTarget*>( &msgtarget ), "addr1" );
    
    st->setDouble( 0.1 );
    EXPECT_EQ( st->getStoredDouble(), 0.1 );
    
    st->setStoredInt( 99 );
    EXPECT_EQ( st->getStoredInt(), 99 );
    
    EXPECT_EQ( st->divide( 10, 5), 2.0 );

    const std::string testString( "1234" );
    st->setString( testString );
    EXPECT_STREQ( testString.c_str() , st->getStoredString().c_str() );
    
    std::vector<co::int32> intVec;
    std::vector<double> doubleVec;
    std::vector<std::string> stringVec;
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 65 + i;
		letter[1] = '\0';
        stringVec.push_back( letter );
        intVec.push_back( i );
        doubleVec.push_back( static_cast<double>( i ) );
    }
    
    st->setStoredIntList( intVec );
    st->setStoredStringList( stringVec );
    st->setStoredDoubleList( doubleVec );
    
    co::Range<const co::int32> intRange = st->getStoredIntList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( intVec[i], intRange[i] );
    
    co::Range<const double> doubleRange = st->getStoredDoubleList();
    for( int i = 0; i < intVec.size(); i++ )
        EXPECT_EQ( doubleVec[i], doubleRange[i] );
    
    co::Range<const std::string> stringRange = st->getStoredStringList();
    for( int i = 0; i < intVec.size(); i++ )
            EXPECT_STREQ( stringVec[i].c_str(), stringRange[i].c_str() );
    
    
    EXPECT_EQ( st->incrementInt( 1 ), 2 );
    EXPECT_DOUBLE_EQ( st->divide( 10, 5 ), 2 );
    EXPECT_STREQ( "abcdef", st->concatenateString( "abc", "def" ).c_str() );
    
    std::vector<std::string> stringVec2;
    for( int i = 0; i < 10; i++ )
    {
        char letter[2];
		letter[0] = 21 + i;
		letter[1] = '\0';
        stringVec2.push_back( letter );
    }
    stringRange = st->getThirdElements( stringVec, stringVec2 );
    
    EXPECT_STREQ( stringRange[0].c_str(), stringVec[2].c_str() );
    EXPECT_STREQ( stringRange[1].c_str(), stringVec2[2].c_str() );
}
    
}