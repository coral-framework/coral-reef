#ifndef __REEF_PASSIVELINK_H__
#define __REEF_PASSIVELINK_H__

#include "PassiveLink_Base.h"

namespace testTransport {

class Transport;

class PassiveLink : public PassiveLink_Base
{
    
public:    
	PassiveLink();
    
    PassiveLink( Transport* creator, const std::string& address );

	virtual ~PassiveLink();
    
    bool bind( const std::string& address );

	// ------ reef.IPassiveLink Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receive( std::string& msg );

	void sendReply( const std::string& msg );

private:
    std::string _address;
    Transport* _creator;
};

} // namespace reef

#endif