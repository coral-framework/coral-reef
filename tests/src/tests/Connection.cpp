#include "network/Connection.h"

#include <string.h>

#include <algorithm>

#include "FakeSocket.h"

namespace reef {

Connecter::Connecter() 
    : _context( 1 ), _socket( _context, ZMQ_DEALER ), _connected( false )
{
    // empty
}

Connecter::~Connecter()
{
    // empty
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
    // empty
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