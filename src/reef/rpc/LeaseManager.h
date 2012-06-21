#ifndef _REEF_LEASEMANAGER_H_
#define _REEF_LEASEMANAGER_H_

#include <co/IObject.h>

#include <map>
#include <set>

namespace reef {
namespace rpc {

class LeaseManager
{
public:
    /* Tries to add a new lease for \param lessorID belonging to \param lesseeEnpoint. Does nothing
     if there is already a lease for \param lesseeEndpoint. */
    void addLease( co::int32 lessorID, const std::string lesseeEndpoint );
    
    /* Removes a lease for \param lessorID belonging to \param lesseeEnpoint. Returns true if there 
     are no more leases for \param lessorID and it has been removed, false otherwise. */
    bool removeLease( co::int32 lessorID, const std::string lesseeEndpoint );
    
private:
    
    /*!
     \brief Class that represents a Lessee, a remote node that holds leases here.
     */
    class Lessee
    {
    public:
        Lessee()
        {
            
        }
        
        inline bool addLease( co::int32 lessorID )
        {
            std::pair<std::set<co::int32>::iterator, bool> res = _ids.insert( lessorID );
            return res.second;
        }
        
        inline bool removeLease( co::int32 lessorID )
        {
            return _ids.erase( lessorID );
        }
        
        inline bool searchLease( co::int32 lessorID )
        {
            _it = _ids.find( lessorID );
            return _it != _ids.end() ? true : false;
        }
        
        inline bool hasLeases()
        {
            return !_ids.empty();
        }
        
    private:
        std::set<co::int32> _ids;
        std::set<co::int32>::iterator _it;
    };

    std::map<std::string, Lessee*> _lessees;
    
    Lessee* findLessee( const std::string& lesseeEndpoint );
};

}
}
#endif