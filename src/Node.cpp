/*
 * Component implementation template for 'reef.Node'.
 */

#include "Node_Base.h"
#include "reef/IRemoteServiceProxy.h"
#include <co/IComponent.h>
#include <co/RefPtr.h>

namespace reef {

class Node : public Node_Base
{
public:
	Node()
	{
		// empty constructor
	}

	virtual ~Node()
	{
		// empty destructor
	}

	// ------ reef.INode Methods ------ //

	reef::IRemoteServiceProxy* createObject( co::IComponent* type )
	{
		static co::RefPtr<reef::IRemoteServiceProxy> dummy;
		return dummy.get();
	}

private:
	// member variables
};

CORAL_EXPORT_COMPONENT( Node, Node );

} // namespace reef
