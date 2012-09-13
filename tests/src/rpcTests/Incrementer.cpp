// Component used to test the synchronization barrier

#include "Incrementer_Base.h"

#include <reef/rpc/INode.h>

#include <co/RefPtr.h>

namespace rpcTests {

class Incrementer : public Incrementer_Base
{
public:
	Incrementer() : _numberOne( 1 )
	{
	}

	virtual ~Incrementer()
	{
	}

	// ------ rpcTests.IIncrementer Methods ------ //

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
	// ------ Receptacle 'node' (reef.rpc.INode) ------ //
    
	reef::rpc::INode* getNodeService()
	{
		return _nodeService.get();
	}
    
	void setNodeService( reef::rpc::INode* nodeService )
	{
		_nodeService = nodeService;
	}
    
private:
	// member variables
	co::int32 _numberOne;
	co::RefPtr<reef::rpc::INode> _nodeService;
    
};

CORAL_EXPORT_COMPONENT( Incrementer, Incrementer );

} // namespace rpcTests
