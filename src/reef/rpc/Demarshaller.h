#ifndef __REEF_DECODER_H__
#define __REEF_DECODER_H__

#include "Marshaller.h"

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef {
namespace rpc {
    
class Message;
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
    REQUEST_CANCEL_LEASE
};

// Helper class. Extracts parameters from a demarshalled invocation.
class ParameterPuller
{
public:
    void pullValue( co::IType* descriptor, co::Any& valueType );
    void pullReference( ReferenceType& refType );
    
    friend class Demarshaller;
private:
    ParameterPuller();
    
    void setInvocation( const Invocation* invocation );
    
    co::int32 _currentParam;
    const Invocation* _invocation;
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
    void getLookup( outString requesterEndpoint, outString lookupKey );
    void getLease( outString requesterEndpoint, co::int32& leaseInstanceID );
    void getCancelLease( outString requesterEndpoint, co::int32& leaseInstanceID );
    
    // If message type is Invocation, then this method gets the parameters
    ParameterPuller& getInvocation( outString requesterEndpoint, InvocationDetails& details );
    
    // If MessageType is Return, then this method retrives the return.
    void getValueTypeReturn( co::IType* descriptor, co::Any& valueAny );
    void getRefTypeReturn( ReferenceType& refType );
    co::int32 getIntReturn();
    
private:
    void checkIfCallMsg();
    
private:
    Message* _message;
    MessageType _msgType;
    Parameter* _parameter;
    co::int32 _currentParam;
    
    ParameterPuller _puller;
    
};
    
}
    
}
#endif
