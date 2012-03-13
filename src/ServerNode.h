#include "ServerNode_Base.h"
#include <map>

namespace reef {

class Binder;
class Servant;
class Decoder;
    
class ServerNode : public ServerNode_Base
{
public:
    ServerNode();
    
    virtual ~ServerNode();
    
    void start( const std::string& address );
    
	void update();

    void stop();
       
    // DecoderChannel
    int newInstance( const std::string& typeName );

	void registerInstance( co::int32 virtualAddress, co::IObject* object );
    
	co::IObject* mapInstance( co::int32 virtualAddress );

private:
	Binder* _binder;
    Decoder* _decoder;

    typedef std::vector<Servant*> Channels;
    
    Channels _channels;
	typedef std::map<co::int32,co::IObject*> InstanceMap;
	InstanceMap _instanceMap;
};
    
} // namespace reef