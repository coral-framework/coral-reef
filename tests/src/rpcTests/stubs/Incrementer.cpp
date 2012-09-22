// Component used to test the synchronization barrier

#include "Incrementer_Base.h"

#include <rpc/INode.h>

#include <co/RefPtr.h>

namespace stubs {

class Incrementer : public Incrementer_Base
{
public:
	Incrementer() : _numberOne( 1 )
	{
	}

	virtual ~Incrementer()
	{
	}

	// ------ stubs.IIncrementer Methods ------ //

	co::int32 getNumberOne()
	{
		return _numberOne;
	}

	void incrementAsync()
	{
        _numberOne++;
	}

	void incrementSync()
	{
		_nodeService->hitBarrier();
        _numberOne++;
	}

protected:
	// ------ Receptacle 'node' (rpc.INode) ------ //
    
	rpc::INode* getNodeService()
	{
		return _nodeService.get();
	}
    
	void setNodeService( rpc::INode* nodeService )
	{
		_nodeService = nodeService;
	}
    
private:
	// member variables
	co::int32 _numberOne;
	co::RefPtr<rpc::INode> _nodeService;
    
};

CORAL_EXPORT_COMPONENT( Incrementer, Incrementer );

} // namespace stubs
