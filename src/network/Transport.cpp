#include "Transport.h"

#include "ZMQTransport.h"
//#include "ListTransport.h"

#include <co/Exception.h>

#include <map>
#include <string.h>
#include <algorithm>

namespace reef {
    
Transport* Transport::_instance = 0;
 
Transport::~Transport()
{
    
}
    
void Transport::setConcreteTransport( Transports concreteType )
{
    if( _instance )
        throw new co::Exception( "Must clear the current transport instance before changing its type" );
    
    switch( concreteType )
    {
        case Transports::ZMQ: _instance = new ZMQTransport(); break;
        case Transports::LIST: throw new co::Exception( "NYI" );
    }
}

Transport* Transport::getInstance()
{
    if( _instance )
        return _instance;
    
    throw new co::Exception( "Must set the concrete Transport type before getting instance" );
}
    
void Transport::clearInstance()
{
    assert( _instance );
    
    delete _instance;
    _instance = 0;
}
    
Connecter* Transport::getConnecter( const std::string& address )
{    
    Connecters::iterator it = _connecters.find( address );
    if( it != _connecters.end() )
    {
        return (*it).second;
    }
    
    Connecter* connecter = createConnecter( address );
    
    return connecter;
}

void Transport::clearConnecter( const std::string& address )
{
    // remove the entry in the connections map
    Connecters::iterator it = _connecters.find( address );
    if( it != _connecters.end() )
    {
        _connecters.erase( it );
    }
}
    
Binder* Transport::newBinder( const std::string& address )
{
    return createBinder( address );
}

} // namespace reef