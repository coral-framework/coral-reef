#ifndef __REEF_DECODER_H__
#define __REEF_DECODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef 
{
    
class Message;
class Message_Member;

class Decoder
{
    
public:
    enum RefOwner
    {
        LOCAL,
        RECEIVER,
        ANOTHER
    };
    
    
    Decoder();
    ~Decoder();
    
    /* 
       Sets a msg for decoding, if instanceID == 0 then it is a New Instance message, 
     else it is a Call msg. Call the appropriate method for decoding each case as explained below. 
     */
    void setMsgForDecoding( const std::string& msg, co::int32& instanceID, bool& hasReturn );
    
    // if msg type is NEW, then, this function will decode it
    void decodeNewInstMsg( std::string& typeName );
    
    /* 
     Starts a decoding state of call/field msg. 
     The decoding state will only be reset after all params are decoded.
     */
    void beginDecodingCallMsg( co::int32& facetIdx, co::int32& memberIdx );
    
    void getValueParam( co::Any& param, co::IType* descriptor );
    
    void getRefParam( co::int32& instanceID, co::int32& facetIdx, RefOwner& owner,
                     std::string& ownerAddress );
    
    
    // ----- Just for plain simple types data, so message structure with it ----- //
    void decodeData( const std::string& msg, bool& value );
    
    void decodeData( const std::string& msg, double& value );
    
    void decodeData( const std::string& msg, co::int32& value );
    
    void decodeData( const std::string& msg, std::string& value );
    
    void decodeData( const std::string& msg, co::IType* descriptor, co::Any& value );
private:
    void checkIfCallMsg();
    
private:
    Message* _message;
    const Message_Member* _msgMember;
    co::int32 _currentParam;
    
};

}
#endif