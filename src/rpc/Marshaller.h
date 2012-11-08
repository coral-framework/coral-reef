#ifndef __RPC_MARSHAL_H__
#define __RPC_MARSHAL_H__

#include "Message.pb.h"
#include <rpc/RemotingException.h>

#include <co/Any.h>
#include <co/Coral.h>
#include <co/IType.h>
#include <co/Exception.h>

#include <string>

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
    void pushValue( const co::Any& valueType, co::IType* descriptor );
    void pushReference( const ReferenceType& refType );
    
    friend class Marshaller;
private:
    
    ParameterPusher();
    ::google::protobuf::RepeatedPtrField< ::rpc::Parameter >* _params;
};
  
enum ExceptionType
{
    EX_CORAL,
    EX_REMOTING,
    EX_STD,
    EX_UNKNOWN
};
    
typedef const std::string& inString;
typedef std::string& outString;
    
/*
    \brief Marshals messages.
 
    Works as a state machine. For marshalling, user needs to call any method that had "marshal" in 
    its name followed by a call to get method, which returns via parameter the marshalled message.
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
    void marshalLookup( inString requesterEndpoint, inString lookupKey, inString instanceType,
                       outString msg );
    void marshalLease( inString requesterEndpoint, co::int32 leaseInstanceID, outString msg );
    
    // Invocations for actual instances
    ParameterPusher& beginInvocation( inString requesterEndpoint, InvocationDetails details );
    void marshalInvocation( outString msg ); 

	ParameterPusher& beginOutput();
	void marshalOutput( outString msg );
    
    // Return values
    void marshalIntReturn( co::int32 value, outString msg );
    
    // Exceptions
    void marshalException( ExceptionType exType, inString exTypeName, inString what, outString msg );
    
    // Synchronization Barrier
    void marshalBarrierUp( inString requesterEndpoint, outString msg );
    void marshalBarrierHit( inString requesterEndpoint, outString msg );
    void marshalBarrierDown( inString requesterEndpoint, outString msg );
    
private:
    Message* _message;
    Request* _request;
    Invocation* _invocation;
    ParameterPusher _paramPusher;
    bool _msgClear;
};

} // namespace rpc

#endif