#include "network/Transport.h"

#include <string.h>

#include <algorithm>

#include "FakeSocket.h"

namespace reef {

typedef std::map<std::string, Connecter*> Connecters;

// map holding all open connections
static Connecters _connecters;

Connecter* Connecter::getOrOpenConnection( const std::string& address )
{    
    Connecters::iterator it = _connecters.find( address );
    if( it != _connecters.end() )
    {
        return (*it).second;
    }
    
    Connecter* connecter = new Connecter();
    connecter->connect( address );
    
    return connecter;
}

Connecter::Connecter() 
    : _context( 1 ), _socket( _context, ZMQ_DEALER ), _connected( false )
{
    // empty
}

Connecter::~Connecter()
{
    if( _connected )
        close();
    
    // remove the entry in the connections map
    Connecters::iterator it = _connecters.find( _address );
    if( it != _connecters.end() )
    {
        _connecters.erase( it );
    }
}
    
bool Connecter::connect( const std::string& address )
{
	_address = address;
	FakeSocket::bindOrConnectAt( address );
	_connected = true;
	return true;
}
    
void Connecter::close()
{
	FakeSocket::closeAt( _address );
    _connected = false;
	_address = "";
}
    
void Connecter::send( const std::string& data )
{
	FakeSocket::sendAt( data, _address );
}

bool Connecter::receiveReply( std::string& data )
{
	FakeSocket::receiveReply( data, _address );

	return true;
}
    
Binder::Binder()
    : _context( 1 ), _socket( _context, ZMQ_ROUTER ), _bound( false )
{
    // empty
}

Binder::~Binder()
{
    if( _bound )
        close();
}

bool Binder::bind( const std::string& address )
{
	FakeSocket::bindOrConnectAt( address );
	_address = address;
    _bound = true;
    return true;
}

void Binder::close()
{
	FakeSocket::closeAt( _address );
    _bound = false;
}

bool Binder::receive( std::string& data )
{
	FakeSocket::receiveAt( data, _address );
	return true;
}

void Binder::reply( const std::string& data )
{
	FakeSocket::reply( data, _address );
}
    
} // namespace reef