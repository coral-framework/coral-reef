#include "Connection.h"

#include <map>
#include <string.h>
#include <algorithm>

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
    _connecters.insert( std::pair<std::string, Connecter*>( address, connecter ) );
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
    // remove the entry in the connections map
    Connecters::iterator it = _connecters.find( _address );
    if( it != _connecters.end() )
    {
        _connecters.erase( it );
    }
}
    
bool Connecter::connect( const std::string& address )
{
    _socket.connect( address.c_str() );
    _connected = true;
    return true;
}
    
void Connecter::close()
{
    _socket.close();
    _connected = false;
}
    
void Connecter::send( const std::string& data )
{
    zmq::message_t msg( data.size() );
    memcpy( msg.data(), data.data(), data.size() );
    _socket.send( msg );
}

bool Connecter::receiveReply( std::string& data )
{
    zmq::message_t msg;
    _socket.recv( &msg );
    data.resize( msg.size() );
    memcpy( &data[0], msg.data(), msg.size() );

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
    _socket.bind( address.c_str() );
    _bound = true;
    return true;
}

void Binder::close()
{
    _bound = false;
    _socket.close();
}

bool Binder::receive( std::string& data )
{
    // check if there is a message and save its sender
    if( _socket.recv( &_lastSender, ZMQ_NOBLOCK ) )
	{
		// get the actual message
		zmq::message_t msg;
		_socket.recv( &msg );
		data.resize( msg.size() );
		memcpy( &data[0], msg.data(), msg.size() );
		return true;
	}
	else if( zmq_errno() == EAGAIN )
	{
		return false;
	}
	else
	{
		//TODO: exception
		assert( false );
		return false;
	}
}

void Binder::reply( const std::string& data )
{
	_socket.send( _lastSender, ZMQ_SNDMORE );

    zmq::message_t msg( data.size() );
    memcpy( msg.data(), data.data(), data.size() );
    _socket.send( msg );
}
    
} // namespace reef