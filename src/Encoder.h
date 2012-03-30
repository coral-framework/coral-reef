#ifndef __REEF_ENCODER_H__
#define __REEF_ENCODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef 
{
    
class Message;
class Message_Member;

class Encoder
{
    
public:
    enum RefOwner
    {
        LOCAL,
        RECEIVER,
        ANOTHER
    };
    
    Encoder();
    ~Encoder();
    
    // ----- New Instance ----- //
    void encodeNewInstMsg( const std::string& typeName, std::string& msg );
    
    
    /*
     Encoder functions for Call/Field msgs. As these messages require different amounts of 
     parameters of different types, they can't be encoded with a single function. 
     Usage: begin, add all the parameters and finish encoding the message. 
     */
    void beginEncodingCallMsg( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx,
                              bool hasReturn );
    
    // adds a Value Type parameter
    void addValueParam( const co::Any& param );
    
    // adds a ref-type parameter, instance is local
    void addRefParam( co::int32 instanceID, co::int32 facetIdx, RefOwner owner, 
                     const std::string* ownerAddress = 0 );
    
    // get the built call msg with all the arguments
    void finishEncodingCallMsg( std::string& msg );
    
    
    // ----- Data Container codec ----- //
    void encodeData( bool value, std::string& msg );
    
    void encodeData( double value, std::string& msg );
    
    void encodeData( co::int32 value, std::string& msg );
    
    void encodeData( const std::string& value, std::string& msg );
    
    void encodeData( const co::Any& value, std::string& msg );
private:
    
    void checkIfCallMsg();
    
private:
    Message* _message;
    Message_Member* _msgMember;
    
};

}
#endif