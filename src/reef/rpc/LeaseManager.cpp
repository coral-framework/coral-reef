#include "LeaseManager.h"

namespace reef {
namespace rpc {

void LeaseManager::addLease( co::int32 lessorID, const std::string lesseeEndpoint )
{
    Lessee* lessee = findLessee( lesseeEndpoint );
    
    if( !lessee )
    {
        lessee = new Lessee();
        _lessees.insert( std::pair<std::string, Lessee*>( lesseeEndpoint, lessee ) );
    }
    
    lessee->addLease( lessorID );
    
}

bool LeaseManager::removeLease( co::int32 lessorID, const std::string lesseeEndpoint )
{
    std::map<std::string, Lessee*>::iterator it = _lessees.find( lesseeEndpoint );
    assert( it != _lessees.end() ); //REMOTINGERROR There is no Lessee associated with ip
    
    Lessee* lessee = it->second;
    if( !lessee->removeLease( lessorID ) )
    {
        assert( false ); //REMOTINGERROR There is no reference to the Id
    }
    
    if( lessee->hasLeases() )
        return false;
    
    _lessees.erase( it );
    delete lessee;
    return true;
}
   
LeaseManager::Lessee* LeaseManager::findLessee( const std::string& lesseeEnpoint )
{
    std::map<std::string, Lessee*>::iterator it = _lessees.find( lesseeEnpoint );
    return  it == _lessees.end() ? 0: it->second;
}
}
}
