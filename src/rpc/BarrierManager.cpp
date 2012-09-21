#include "BarrierManager.h"
#include "RequestorManager.h"

#include <cassert>

namespace rpc {

BarrierManager::BarrierManager( RequestorManager* reqMan ) : _reqMan( reqMan ), _capacity( 0 ),
        _numHits( 0 )
{
}

void BarrierManager::raiseBarrier( int capacity )
{
    assert( capacity > 0 );
    
    _capacity = capacity;
    
    _reqMan->broadcastBarrierUp();
}

void BarrierManager::onBarrierHit()
{
    assert( _capacity > 0 ); // Someone sends a hit msg without a barrier (inconsistency)
    
    if( ++_numHits >= _capacity )
    {
        _capacity = 0;
        _numHits = 0;
        _reqMan->broadcastBarrierDown();
    }
}

} // namespace rpc