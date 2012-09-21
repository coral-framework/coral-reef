#ifndef _RPC_BARRIERMANAGER_H_
#define _RPC_BARRIERMANAGER_H_

namespace rpc {

class RequestorManager;
    
class BarrierManager
{
public:
    BarrierManager( RequestorManager* reqMan );
    
    // Creates a barrier that holds until \a capacity nodes hit it.
    void raiseBarrier( int capacity );
    
    /* This method is triggered upon the receival of a msg from a node that just hit the barrier. 
     If barrier capacity is reached, a message will be broadcast telling that the barrier is down. */
    void onBarrierHit();
    
    inline bool isBarrierUp() { return _capacity; }
private:
    RequestorManager* _reqMan;
    
    int _capacity;
    int _numHits;
};
  
} // namespace rpc

#endif