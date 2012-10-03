#include "LocalSpaceObserver.h"

void LocalSpaceObserver::onGraphChanged( ca::IGraphChanges* changes )
{
	_lastChanges.push_back( changes );
}

co::IInterface* LocalSpaceObserver::getInterface() { return 0; }
co::IObject* LocalSpaceObserver::getProvider() { return 0; }
co::IPort* LocalSpaceObserver::getFacet() { return 0; }
void LocalSpaceObserver::serviceRetain() {;}
void LocalSpaceObserver::serviceRelease() {;}