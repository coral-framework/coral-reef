#ifndef __REEF_ENCODER_H__
#define __REEF_ENCODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef {
namespace rpc {

    
class Message;
class Message_Member;

class Marshaller
{
    
public:
    enum RefOwner
    {
        LOCAL,
        RECEIVER,
        ANOTHER
    };
    
    Marshaller();
    ~Marshaller();
    
    // Marshals a request for a new instance
    void marshalNewInstance( const std::string& typeName, const std::string& referer, 
                            std::string& marshalledRequest );
    
    // Marshals a request for access to an instance
    void marshalAccessInstance( co::int32 instanceID, bool increment, const std::string& referer, 
                               std::string& marshalledRequest );
    
    // Marshals a request for an existing instance
    void marshalFindInstance( const std::string& key, const std::string& referer, 
                             std::string& marshalledRequest );
    
    /*
     Marshaller functions for Call/Field requests. As these requests require different amounts of 
     parameters of different types, they can't be marshalled with a single call. Therefore, the
     begin method starts an internal state of request marshalling, then, after every argument is added,
     the getMarshalledCall method should be called to get the ready request, also, the state is reset.
     */
    // Starts an internal state of call request marshalling, must be matched with a getMarshalledCall
    void beginCallMarshalling( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx,
                              bool hasReturn );
    
    // adds a Value Type parameter
    void marshalValueParam( const co::Any& param );
    
    // adds a ref-type parameter, instance is local
    void marshalRefParam( co::int32 instanceID, co::int32 facetIdx, RefOwner owner, 
                     const std::string* instanceType = 0, const std::string* ownerAddress = 0 );
    
    // gets the marshalled call request with all the arguments
    void getMarshalledCall( std::string& marshalledRequest );
    
    
    // ----- Encodes plain data messages ----- //
    void marshalData( bool value, std::string& marshalledData );
    
    void marshalData( double value, std::string& marshalledData );
    
    void marshalData( co::int32 value, std::string& marshalledData );
    
    void marshalData( const std::string& value, std::string& marshalledData );
    
    void marshalData( const co::Any& value, std::string& marshalledData );
private:
    
    void checkIfCallMsg();
    
private:
    Message* _message;
    Message_Member* _msgMember;
    
};
    
}
    
}
#endif