#ifndef __REEF_MARSHAL_H__
#define __REEF_MARSHAL_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef {
namespace rpc {

class Message;
class Request;
class Invocation;
       
enum InstanceOwner
{
    OWNER_SENDER,
    OWNER_RECEIVER,
    OWNER_ANOTHER
};

struct ReferenceType
{
    co::int32 instanceID;
    co::int32 facetIdx;
    InstanceOwner owner;
    std::string instanceType;
    std::string ownerEndpoint;
};

struct InvocationDetails
{
    co::int32 instanceID; 
    co::int32 facetIdx;
    co::int32 memberIdx;
    co::int32 typeDepth; 
    bool hasReturn;

    InvocationDetails() {}
    
    InvocationDetails( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx, 
                      co::int32 typeDepth, bool hasReturn )
    {
        this->instanceID = instanceID;
        this->facetIdx = facetIdx;   
        this->memberIdx = memberIdx;
        this->typeDepth = typeDepth;
        this->hasReturn = hasReturn;
    }
};
    
class ParameterPusher
{
public:    
    void pushValue( const co::Any& valueType );
    void pushReference( const ReferenceType& refType );
    
    friend class Marshaller;
private:
    
    ParameterPusher();
    Invocation* _invocation;
};
    
typedef const std::string& inString;
typedef std::string& outString;
    
/*
    \brief Marshals and demarshals messages.
 
    Works as a state machine. For marshalling, user needs to call any method that had "marshal" in 
    its name followed by a call to get method, which returns via parameter the marshalled message.
    For demarshalling, the user needs to set a marshalled message by calling the "demarshal" method.
    Then, call the appropriate "get" method based on the MessageType returned by "demarshal".
*/
class Marshaller
{
public:
    
    Marshaller();
    ~Marshaller();
    
    //
    // Marshalling methods. Receive the parameters and return marshalled msg by out parameter
    //
    
    // Requests for the instanceManager
    void marshalNew( inString requesterEndpoint, inString instanceType, outString msg );
    void marshalLookup( inString requesterEndpoint, inString lookupKey, outString msg );
    void marshalLease( inString requesterEndpoint, co::int32 leaseInstanceID, outString msg );
    void marshalCancelLease( inString requesterEndpoint, co::int32 leaseInstanceID, outString msg );
    
    // Invocations for actual instances
    ParameterPusher& beginInvocation( inString requesterEndpoint, InvocationDetails details );
    void marshalInvocation( outString msg ); 
    
    // Return values
    void marshalIntReturn( co::int32 value, outString msg );
    void marshalValueTypeReturn( const co::Any& valueAny, outString msg );
    void marshalRefTypeReturn( const ReferenceType& refType, outString msg );
    
private:
    Message* _message;
    Request* _request;
    Invocation* _invocation;
    ParameterPusher _paramPusher;
    bool _msgClear;
};

}
}

#endif