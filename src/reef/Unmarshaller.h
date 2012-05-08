#ifndef __REEF_DECODER_H__
#define __REEF_DECODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef 
{
    
class Message;
class Message_Member;

    
/*
    Unmarshals remote requests and data messages. Requests are disassembled in multiple steps. 
    First, a request is set. Then, by using the request's meta information returned by the set method,
    the actual request is unmarshalled and its information returned through 1+nArguments calls.
 */
class Unmarshaller
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
    
    Unmarshaller();
    ~Unmarshaller();
    
    /* 
        Sets a request for unmarshalling. Returns request meta information. User must call the 
        appropriate method for unmarshaling based on meta information. Each case is explained below. 
     */
    void setMarshalledRequest( const std::string& request, MsgType& type, co::int32& instanceID,
                           bool& hasReturn, std::string* referer = 0 );
    
    // Unmarshals a new instance request. typeName is the instance's component name return value
    void unmarshalNewInstance( std::string& typeName );
    
    // Unmarshals an access instance request.  Increment means if requesting or revoking access 
    void unmarshalAccessInstance( co::int32& instanceID, bool& increment );
    
    // Unmarshals a find instance request. A local instance must be published under key.
    void unmarshalFindInstance( std::string& key );

    /*
     Unmarshaller functions for Call/Field requests. As these requests require different amounts of 
     parameters of different types, they can't be unmarshalled with a single call. Therefore, the
     begin method starts an internal state of call unmarshalling, then, each parameter must be 
     unmarshalled with an appropriate call.
     */
    void beginUnmarshallingCall( co::int32& facetIdx, co::int32& memberIdx );
    
    // Unmarshals a value type parameter
    void unmarshalValueParam( co::Any& param, co::IType* descriptor );
    
    // Unmarshals a reference type parameter
    void unmarshalRefParam( co::int32& instanceID, co::int32& facetIdx, RefOwner& owner,
                     std::string& instanceType, std::string& ownerAddress );
    
    
    // ----- Just for plain simple types data, so message structure with it ----- //
    void unmarshalData( const std::string& marshalledData, bool& value );
    
    void unmarshalData( const std::string& marshalledData, double& value );
    
    void unmarshalData( const std::string& marshalledData, co::int32& value );
    
    void unmarshalData( const std::string& marshalledData, std::string& value );
    
    void unmarshalData( const std::string& marshalledData, co::IType* descriptor, co::Any& value );
private:
    void checkIfCallMsg();
    
private:
    Message* _message;
    MsgType _msgType;
    const Message_Member* _msgMember;
    co::int32 _currentParam;
    
};

}
#endif
