#ifndef __REEF_MESSAGE_BUILDER_H__
#define __REEF_MESSAGE_BUILDER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef 
{
    
class Message;
class Message_Member;
class RefType;

class MessageEncoder
{
    
public:
    
    enum InstanceOwner
    {
        SELF,
        RECEIVER,
        ANOTHER
    };
    
    MessageEncoder();
    ~MessageEncoder();
    // ----- New Instance msg functions ----- //
    void newNewInstanceMsg( const std::string& typeName, std::string& msg );
    
    
    // ----- Call and Field (Member) msg Functions ----- //
    // starts an internal construction of a call/field msg
    void buildCallMsg( co::int32 instanceID, co::int32 facetIdx, co::int32 memberIdx );
    
    // adds a Value Type parameter
    void addValueParam( const co::Any param );
    
    // adds a ref-type parameter, instance is local
    void addLocalRefParam( co::int32 instanceID, co::int32 facetIdx );
    
    // adds a ref-type parameter, instance is on the receiver
    void addReceiverRefParam( co::int32 instanceID, co::int32 facetIdx );
    
    // adds a ref-type parameter, instance is on another host (requires the host's address)
    void addAnotherRefParam( co::int32 instanceID, co::int32 facetIdx, 
                            const std::string& ownerAddress );
    
    // get the built call msg with all the arguments
    void getBuiltCallMsg( std::string& msg );
    
    
    // ----- Data Container msg Functions ----- //
    void newDataMsg( bool value, std::string& msg );
    void newDataMsg( double value, std::string& msg );
    void newDataMsg( co::int32 value, std::string& msg );
    void newDataMsg( const std::string& value, std::string& msg );
    
private:
    // fill all common fields for ref params and returns the RefType missing just the owner field
    RefType* makeRefParam( co::int32 instanceID, co::int32 facetIdx );
    
    void checkIfCallMsg();
    
private:
    Message* _message;
    Message_Member* _msgMember;
    
};

}
#endif