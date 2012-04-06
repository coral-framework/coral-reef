#include "ClientNode.h"
#include "RemoteObject.h"
#include "Encoder.h"
#include "network/Connection.h"

#include <co/RefPtr.h>
#include <co/IObject.h>

#include <map>

namespace reef {

    ClientNode::ClientNode()
    {
        // empty constructor
    }
    
    ClientNode::~ClientNode()
    {
    }
    
    co::IObject* ClientNode::newRemoteInstance( const std::string& componentTypeName, 
                                   const std::string& address )
    {
        Connecter* connecter = Connecter::getOrOpenConnection( address );
        co::int32 instanceID = requestNewInstance( connecter, componentTypeName );
        
        co::IComponent* component = co::cast<co::IComponent>( 
                                          co::getType( componentTypeName ) );
        
        return RemoteObject::getOrCreateRemoteObject( component, connecter, instanceID );
    }
    
    co::IObject* getRemoteObjectFor( co::IComponent* component, co::int32 instanceID, 
                                    const std::string& ownerAddress )
    {
        Connecter* connecter = Connecter::getOrOpenConnection( ownerAddress );
        
        return RemoteObject::getOrCreateRemoteObject( component, connecter, instanceID );
    }
    
    co::int32 ClientNode::requestNewInstance( Connecter* connecter, const std::string& componentName )
    {
        std::string msg;
        _encoder.encodeNewInstMsg( componentName, msg );
        connecter->send( msg );
        
        connecter->receiveReply( msg );
        
        co::int32 instanceID;
        _decoder.decodeData( msg, instanceID );
        
        return instanceID;
    }

CORAL_EXPORT_COMPONENT( ClientNode, ClientNode );
    
} // namespace reef