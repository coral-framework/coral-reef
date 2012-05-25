#ifndef __REEF_DECODER_H__
#define __REEF_DECODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef {
namespace rpc {

    
class Message;
class Argument;
class Message_Member;

    
/*
    Refer to Marshaller class in order to understand the parameters of the methods.
    Demarshals remote requests and data messages. Requests are disassembled in multiple steps. 
    First, a request is set. Then, by using the request's meta information returned by the set method,
    the actual request is demarshalled and its information returned through 1+nArguments calls.
 */
class Demarshaller
{
    
public:
    enum RefOwner
    {
        LOCAL,
        RECEIVER,
        ANOTHER
    };
    
    enum MsgType
    {
        NEW_INST,
        ACCESS_INST,
        FIND_INST,
        CALL
    };
    
    Demarshaller();
    ~Demarshaller();
    
    /* 
        Sets a request for demarshalling. Returns request meta information. User must call the 
        appropriate method for demarshaling based on meta information. Each case is explained below.
        \type Set implicitly by the Marshaller depending on the type of request.
     */
    void setMarshalledRequest( const std::string& request, MsgType& type, co::int32& instanceId,
                           bool& hasReturn );
    
    // Demarshals a new instance request. typeName is the instance's component name return value
    void demarshalNewInstance( std::string& typeName, std::string& referer );
    
    // Demarshals an access instance request.  Increment means if requesting or revoking access 
    void demarshalAccessInstance( co::int32& instanceId, bool& increment, std::string& referer );
    
    // Demarshals a find instance request. A local instance must be published under key.
    void demarshalFindInstance( std::string& key, std::string& referer );

    /*
     Demarshaller functions for Call/Field requests. As these requests require different amounts of 
     parameters of different types, they can't be demarshalled with a single call. Therefore, the
     begin method starts an internal state of call demarshalling, then, each parameter must be 
     demarshalled with an appropriate call.
     */
    void beginDemarshallingCall( co::int32& facetIdx, co::int32& memberIdx, co::int32& typeDepth,
                                std::string& caller );
    
    // Demarshals a value type parameter from the current call 
    void demarshalValueParam( co::Any& param, co::IType* descriptor );
    
    // Demarshals a reference type parameter from the current call
    void demarshalReferenceParam( co::int32& instanceId, co::int32& facetIdx, RefOwner& owner,
                     std::string& instanceType, std::string& ownerAddress );

    
    // Demarshals a rogue reference type
    void demarshalReference( const std::string& data, co::int32& instanceId, co::int32& facetIdx, 
                            RefOwner& owner, std::string& instanceType, std::string& ownerAddress );
    
    // Demarshals a rogue value type
    void demarshalValue( const std::string& data, co::IType* descriptor, co::Any& value );
    
    // ----- Just for plain simple types data, so message structure with it ----- //
    void demarshalData( const std::string& data, bool& value );
    
    void demarshalData( const std::string& data, double& value );
    
    void demarshalData( const std::string& data, co::int32& value );
    
    void demarshalData( const std::string& data, std::string& value );
    
private:
    void checkIfCallMsg();
    
private:
    Message* _message;
    MsgType _msgType;
    Argument* _argument;
    const Message_Member* _msgMember;
    co::int32 _currentParam;
    
};
    
}
    
}
#endif
