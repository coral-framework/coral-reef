#include "LeaseManager.h"

namespace reef {
namespace rpc {

LeaseManager::~LeaseManager()
{
    std::map<std::string, Lessee*>::iterator it = _lessees.begin();
    
    for( ; it != _lessees.end(); it++ )
    {
        delete it->second;
    }
}
    
void LeaseManager::addLease( co::int32 lessorID, const std::string lesseeEndpoint )
{
    Lessee* lessee = findLessee( lesseeEndpoint );
    
    if( !lessee )
    {
        lessee = new Lessee();
        _lessees.insert( std::pair<std::string, Lessee*>( lesseeEndpoint, lessee ) );
    }
    
    lessee->addLease( lessorID );
    
    std::map<co::int32, Lessor*>::iterator it = _lessors.find( lessorID );
    
    Lessor* lessor = it->second;
    if( it == _lessors.end() ) 
    {
        lessor = new Lessor();
        _lessors.insert( std::pair<co::int32, Lessor*>( lessorID, lessor ) );
    }
    lessor->addLease( lessee );
}

bool LeaseManager::removeLease( co::int32 lessorID, const std::string lesseeEndpoint )
{
    std::map<std::string, Lessee*>::iterator lesseeIt = _lessees.find( lesseeEndpoint );
    assert( lesseeIt != _lessees.end() ); //REMOTINGERROR There is no Lessee associated with ip
    
    // Remove the reference to the lessor from the lessee, and if empty, remove the lessee
    Lessee* lessee = lesseeIt->second;
    
    if( !lessee->removeLease( lessorID ) )
        assert( false ); //REMOTINGERROR There is no reference to the Id
    
    if( !lessee->hasLeases() )
    {
        _lessees.erase( lesseeIt );
        delete lessee;
    }
    
    // Find the lessor, remove the reference to the lessee from it
    std::map<co::int32, Lessor*>::iterator lessorIt = _lessors.find( lessorID );
    assert( lessorIt != _lessors.end() ); 
    
    Lessor* lessor = lessorIt->second;
    
    if( !lessor->removeLease( lessee ) )
        assert( false ); //REMOTINGERROR There is no reference to the Id
    
    // Check if lessor is empty, if true, remove the lessor and return true
    if( !lessor->numLeases() )
    {
        _lessors.erase( lessorIt );
        delete lessor;    
        return true;
    }
    
    return false;
}
    
co::int32 LeaseManager::numLeases( co::int32 lessorID )
{
    std::map<co::int32, Lessor*>::iterator lessorIt = _lessors.find( lessorID );
    if( lessorIt != _lessors.end() )
        return lessorIt->second->numLeases();
    
    return 0;
}
    
Lessee* LeaseManager::findLessee( const std::string& lesseeEnpoint )
{
    std::map<std::string, Lessee*>::iterator it = _lessees.find( lesseeEnpoint );
    return  it == _lessees.end() ? 0: it->second;
}
}
}
