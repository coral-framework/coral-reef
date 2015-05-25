#ifndef __RPC_DECODER_H__
#define __RPC_DECODER_H__

#include "Marshaller.h"

#include "Message.pb.h"

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace rpc {
    
class Message;
class Container;
class Parameter;
class Invocation;

// All the possible message types
enum MessageType
{
    INVOCATION,
    RETURN,
    REQUEST_NEW,
    REQUEST_LOOKUP,
    REQUEST_LEASE,
    EXCEPTION,
    BARRIER_UP,
    BARRIER_HIT,
    BARRIER_DOWN,
    INVALID
};

// Helper class. Extracts parameters from a demarshalled invocation.
class ParameterPuller
{
public:
    void pullValue( co::IType* descriptor, const co::Any& ret );
    void pullReference( ReferenceType& refType );
    
    friend class Demarshaller;
private:
    ParameterPuller();
    
    void setParams( const ::google::protobuf::RepeatedPtrField< ::rpc::Parameter >* params );
    
    co::int32 _currentParam;
    const ::google::protobuf::RepeatedPtrField< ::rpc::Parameter >* _params;
};
  
class Demarshaller
{
    
public:
    
    Demarshaller();
    ~Demarshaller();
    
    //
    // Demarshalling methods. Usage: Set the marshalled data first, then get the parameter results.
    //
    
    // Set the data that will be demarshalled and then have its parameters retrieved.
    MessageType demarshal( inString data );
    
    void getNew( outString requesterEndpoint, outString instanceType );
    void getLookup( outString requesterEndpoint, outString lookupKey, outString instanceType );
    void getLease( outString requesterEndpoint, co::int32& leaseInstanceID );
    
    // If message type is Invocation, then this method gets the parameters
    ParameterPuller& getInvocation( outString requesterEndpoint, InvocationDetails& details );
    
	ParameterPuller& getOutput();
    co::int32 getIntReturn();
    
    ExceptionType getException( outString exTypeName, outString what );
    
    // Synchronization Barrier
    void getBarrierCreator( outString creatorEndpoint );
    
private:
    void checkIfCallMsg();
    
private:
    Message* _message;
    MessageType _msgType;
    Parameter* _parameter;
    co::int32 _currentParam;
    
    ParameterPuller _puller;
    
};

} // namespace rpc
#endif
