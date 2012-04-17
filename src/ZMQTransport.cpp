#include "ZMQTransport.h"

#include "ZMQActiveLink.h"
#include "ZMQPassiveLink.h"

#include <reef/IActiveLink.h>
#include <reef/IPassiveLink.h>

namespace reef {
    
ZMQTransport::ZMQTransport()
{
    // empty constructor
}

ZMQTransport::~ZMQTransport()
{
    // empty destructor
}

// ------ reef.ITransport Methods ------ //

IPassiveLink* ZMQTransport::bind( const std::string& addressToListen )
{
    ZMQPassiveLink* link = new ZMQPassiveLink();
    link->bind( addressToListen );
    return link;
}

IActiveLink* ZMQTransport::connect( const std::string& addressToConnect )
{
    // Tries to find an existing ActiveLink and returns it
    ActiveLinks::iterator it = _activeLinks.find( addressToConnect );
    if( it != _activeLinks.end() )
    {
        return it->second;
    }
    
    // Creates a new ALink            
    return createActiveLink( addressToConnect );
}

void ZMQTransport::onLinkDestructor( const std::string& address )
{
    // remove the entry in the connections map
    ActiveLinks::iterator it = _activeLinks.find( address );
    if( it != _activeLinks.end() )
    {
        _activeLinks.erase( it );
    }
}

IActiveLink* ZMQTransport::createActiveLink( const std::string& address )
{
    ZMQActiveLink* link = new ZMQActiveLink( this );
    link->connect( address );
    _activeLinks.insert( std::pair<std::string, IActiveLink*>( address, link ) );
    return link;
}
     
CORAL_EXPORT_COMPONENT( ZMQTransport, ZMQTransport );
    
} // namespace reef
