/*
 * Component implementation template for 'reef.Node'.
 */
#ifndef NODE_H
#define NODE_H

#include "Node_Base.h"
#include <co/RefVector.h>

namespace co {
class IComponent;
}

namespace reef {

class IProxy;
class Proxy;
class Servant;

class Node : public Node_Base
{
public:
	Node();

	virtual ~Node();

	// ------ reef.INode Methods ------ //

	reef::IProxy* createObject( co::IComponent* type );
    
    // Callback for notification of proxy being destroyed
    void onProxyDestruction( co::int32 channel );

private:
	// member variables
	std::vector<Proxy*> _proxies;
	std::vector<Servant*> _servants;
    
    std::vector<std::string> _addressTable;
};


} // namespace reef

#endif