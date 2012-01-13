/*
 * Component implementation template for 'reef.Proxy'.
 */

#include "Proxy.h"
#include "Node.h"
#include "Networking.h"

namespace reef {

	Proxy::Proxy()
	{
		// empty constructor
	}
	
    Proxy::Proxy( reef::Node* node, co::int32 channel ) :
		_node( node ), _channel( channel )
	{
	}

	Proxy::~Proxy()
	{
		if( _node ) 
        {
            _node->onProxyDestruction( _channel );
        }
	}
    
    void Proxy::callMethodById( co::int32 id )
    {
        
    }
    
	void Proxy::callMethodByName( const std::string& name )
    {
        
    }

	void Proxy::receiveMsg( Message* msg )
	{
	}

    CORAL_EXPORT_COMPONENT( Proxy, Proxy );
    
} // namespace reef
