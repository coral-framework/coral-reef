#ifndef __LOCAL_SPACE_OBSERVER__
#define __LOCAL_SPACE_OBSERVER__

#include <co/Coral.h>

#include <ca/IGraphChanges.h>
#include <ca/IGraphObserver.h>

class LocalSpaceObserver : public ca::IGraphObserver
{
public:
	
	ca::IGraphChanges* getLastChanges()
	{
		return _lastChanges.back().get();
	}
	
	void onGraphChanged( ca::IGraphChanges* changes );

	co::IInterface* getInterface();
	co::IObject* getProvider();
	co::IPort* getFacet();
	void serviceRetain();
	void serviceRelease();
	

private:
	std::vector<ca::IGraphChangesRef> _lastChanges;
		
};





#endif

