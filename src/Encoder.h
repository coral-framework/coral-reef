#ifndef _REEF_ENCODER_H_
#define _REEF_ENCODER_H_

#include <co/Any.h>
#include <co/Range.h>
#include <reef/IServerNode.h>

namespace reef
{
    
class Message;
class Argument;
class Connecter;
class Message_Member;
    
/*
    Client-side Channel. Receives the calls from the RemoteObject, 
    encodes and sends to Server
 */
class Encoder
{
public:
    /* 
     Requires a ServerNode in order to publish any ref type that is passed as 
     a parameter.
     */
    Encoder( Connecter* connecter, IServerNode* publisher );
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
    
    // Functions are public for testing
    void makeCallMessage( co::int32 destination, bool hasReturn, Message& owner,
                         co::int32 serviceId, co::int32 methodIndex, 
                         co::Range<co::Any const> args );
    
	void makeSetFieldMessage( co::int32 destination, Message& owner, 
                             co::int32 serviceId, co::int32 fieldIndex, 
                             const co::Any& value );
    
	void makeGetFieldMessage( co::int32 destination, Message& owner, 
                             co::int32 serviceId, co::int32 fieldIndex );

private:
    void write( const Message* message );
    
	void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );

    void publishRefTypes( co::IMethod* method, 
            co::Range<co::Any const> args, std::vector<co::int32>& vAddresses );

    
    void convertArgs( Message_Member* msgMember, co::Range<co::Any const> args );
    
    void convertRefTypeArg( const co::Any refType, Argument* PBArg );
    
    
private:
    co::RefPtr<Connecter> _connecter;
    
    IServerNode* _publisher;
    
    co::int32 _instanceAddress;
};

}

#endif