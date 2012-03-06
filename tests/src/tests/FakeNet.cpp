#include "FakeNet.h"
#include <co/Exception.h>
#include <map>

namespace reef
{
    static std::map<std::string, const Binder*> _boundHosts;
    const Binder* FakeNet::getBinder( const std::string& address )
    {
        std::map<std::string, const Binder*>::iterator it = _boundHosts.find( address );
        if( it == _boundHosts.end() )
            return 0;
        
        return (*it).second;
    }
    
    void FakeNet::bind( const Binder* binder, const std::string& address )
    {
        _boundHosts.insert( std::pair<std::string, const Binder*>( address, binder ) );
    }
}