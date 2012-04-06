#include "ClientNode_Base.h"

#include "RemoteObject.h"

#include <co/IObject.h>

#include <map>

namespace reef {
    
    class IServerNode;
    
    class ClientNode : public ClientNode_Base
    {
    public:
        ClientNode();
        virtual ~ClientNode();
        
        co::IObject* newRemoteInstance( const std::string& componentTypeName, 
                                       const std::string& address );
        
        // Fetch a existing remoteObject pointing to the instance or create a new one
        co::IObject* getRemoteObjectFor( co::int32 instanceID, const std::string& ownerAddress );
        
    private:
        co::int32 requestNewInstance( Connecter* connecter, const std::string& componentName );
        
        Encoder _encoder;
        Decoder _decoder;
    };
    
} // namespace reef