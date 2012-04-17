#ifndef __REEF_ACTIVELINK_H__
#define __REEF_ACTIVELINK_H__

#include "ActiveLink_Base.h"

namespace testTransport {
    
class Transport;

class ActiveLink : public ActiveLink_Base
{
public:
    // The \a creator wil be notified upon the destruction of this object.
    ActiveLink( Transport* creator, const std::string& address );
    
	ActiveLink();

	virtual ~ActiveLink();

	// ------ reef.IActiveLink Methods ------ //

	const std::string& getAddress() { return _address; }

	bool receiveReply( std::string& msg );

	void send( const std::string& msg );

private:
    std::string _address;
    Transport* _creator;
};

} // namespace reef

#endif