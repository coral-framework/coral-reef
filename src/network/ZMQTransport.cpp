#include "ZMQTransport.h"

#include <zmq.hpp>

#include <string.h>
#include <iostream>
#include <algorithm>

namespace reef {
   
class ZMQConnecter : public Connecter
{
public:
    
    // Receives
    ZMQConnecter( const std::string& addressToConnect, Transport* creator ) : _context( 1 ), 
                                        _socket( _context, ZMQ_DEALER ), _connected( false )
    {
        _creator = creator;
        connect( addressToConnect );
    }
    
    ~ZMQConnecter()
    {
        close();
        _creator->clearConnecter( _address );
    }
    
    void send( const std::string& data )
    {
        zmq::message_t msg( data.size() );
        memcpy( msg.data(), data.data(), data.size() );
        _socket.send( msg );
    }
    
    bool receiveReply( std::string& data )
    {
        zmq::message_t msg;
        if( _socket.recv( &msg, ZMQ_NOBLOCK ) )
        {
            data.resize( msg.size() );
            memcpy( &data[0], msg.data(), msg.size() );
            return true;
        }
        
        return false;        
    }
    
    const std::string& getAddress() { return _address; }
    
private:
    bool connect( const std::string& address )
    {
        _address = address;
        _socket.connect( address.c_str() );
        _connected = true;
        return true;
    }
    
    void close()
    {
        _socket.close();
        _connected = false;
    }

private:
    std::string _address;
	zmq::context_t _context;
    zmq::socket_t _socket;
    
    bool _connected;
    
    Transport* _creator;

};
    
   
class ZMQBinder : public Binder
{
public:
    ZMQBinder( const std::string& addressBoundTo )
    : _context( 1 ), _socket( _context, ZMQ_ROUTER ), _bound( false )
    {
        bind( addressBoundTo );
    }
    
    ~ZMQBinder()
    {
        close();
    }
        
    bool receive( std::string& data )
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
    
    void reply( const std::string& data )
    {
        _socket.send( _lastSender, ZMQ_SNDMORE );
        
        zmq::message_t msg( data.size() );
        memcpy( msg.data(), data.data(), data.size() );
        _socket.send( msg );
    }
    
    bool isBound() { return _bound; }
    
    const std::string& getAddress() { return _address; }
    
private:
    
    bool bind( const std::string& address )
    {
        _address = address;
        _socket.bind( address.c_str() );
        _bound = true;
        return true;
    }
    
    void close()
    {
        _address = "";
        _bound = false;
        _socket.close();
    }

private:
    
    std::string _address;
    zmq::context_t _context;
    zmq::socket_t _socket;
    
    zmq::message_t _lastSender;
    std::string _senderAddress;
    
    bool _bound;
};

Connecter* ZMQTransport::createConnecter( const std::string& addressToConnect ) 
{ return new ZMQConnecter( addressToConnect, this ); }
    
Binder* ZMQTransport::createBinder( const std::string& addressBoundTo ) 
{ return new ZMQBinder( addressBoundTo ); }
    
}
