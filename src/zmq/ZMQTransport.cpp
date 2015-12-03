#include "ZMQTransport.h"

#include "ZMQConnector.h"
#include "ZMQAcceptor.h"

#include <rpc/IConnector.h>
#include <rpc/IAcceptor.h>
#include <rpc/INetworkNode.h>

#include <zmq_utils.h>

#include <unordered_set>

#include <co/Coral.h>

#pragma comment(lib, "Ws2_32.lib")

namespace zmq {
    
const int PING_PORT_NUMBER = 9123;
const int SOCKET_POLL_TIMEOUT = 3000;
const int PING_INTERVAL = 200;

bool ZMQTransport::_s_winsockInitialized = false;

ZMQTransport::ZMQTransport() : _context( 1 )
{
    // empty constructor
}

ZMQTransport::~ZMQTransport()
{
    // empty destructor
}

// ------ rpc.ITransport Methods ------ //

rpc::IAcceptor* ZMQTransport::bind( const std::string& addressToListen )
{
    ZMQAcceptor* link = new ZMQAcceptor( _context );
    link->bind( addressToListen );
    return link;
}

rpc::IConnector* ZMQTransport::connect( const std::string& addressToConnect )
{
    ZMQConnector* link = new ZMQConnector( _context );
    link->connect( addressToConnect );
    return link;
}

void ZMQTransport::initWinSock()
{
	WSADATA wsaData;
	int nResult = 0;
	if (!_s_winsockInitialized)
	{
		nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (nResult != NO_ERROR)
		{
			return;
		}
		_s_winsockInitialized = true;
	}
}

bool ZMQTransport::sendAutoDiscoverSignal( const std::string& ipmask, int port )
{
	int nOptOffVal = 0;
	int nOptOnVal = 1;
	int nOptLen = sizeof(int);

	// Initialize Winsock
	initWinSock();

	// Create UDP socket
	SOCKET fdSocket;
	fdSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (fdSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Ask operating system to let us do broadcasts from socket
	int nResult = setsockopt(fdSocket, SOL_SOCKET, SO_BROADCAST, (char *)&nOptOnVal, nOptLen);
	if (nResult != NO_ERROR)
	{
		return false;
	}

	// Set up the sockaddr structure
	struct sockaddr_in saBroadcast = { 0 };
	saBroadcast.sin_family = AF_INET;
	saBroadcast.sin_port = htons(PING_PORT_NUMBER);
	saBroadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	int size = sizeof(saBroadcast);
	if( !WSAStringToAddress( (LPSTR)ipmask.c_str( ), AF_INET, NULL, (struct sockaddr *)&saBroadcast, &size) == 0 )
		return false;
	
		// Broadcast 5 beacon messages
	for (int i = 0; i < 5; i++)
	{
		std::string portStr = std::to_string( (long long)port );
		int bytes = sendto( fdSocket, portStr.c_str( ), portStr.size(), 0, (sockaddr*)&saBroadcast, sizeof(struct sockaddr_in) );
		if (bytes == SOCKET_ERROR)
		{
			return false;
		}
		Sleep(PING_INTERVAL);
	}

	closesocket(fdSocket);

	return true;
}

bool ZMQTransport::discoverRemoteInstances( std::vector<rpc::INetworkNodeRef>& instances, co::uint32 timeout )
{
	int nResult = 0;
	int nOptOffVal = 0;
	int nOptOnVal = 1;
	int nOptLen = sizeof(int);
	
	// Initialize Winsock
	initWinSock();

	//  Create UDP socket
	SOCKET fdSocket;
	fdSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fdSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Set up the sockaddr structure
	struct sockaddr_in saListen = { 0 };
	saListen.sin_family = AF_INET;
	saListen.sin_port = htons(PING_PORT_NUMBER);
	saListen.sin_addr.s_addr = htonl(INADDR_ANY);

	//  Bind the socket
	nResult = ::bind(fdSocket, (sockaddr*)&saListen, sizeof(saListen));
	if (nResult != NO_ERROR)
	{
		return false;
	}

	std::unordered_multiset<std::string> foundIps;

	void* watch = 0;
	int elapsed = 0;
	do
	{
		watch = zmq_stopwatch_start();

		//  Poll socket for a message
		zmq::pollitem_t items[] = { NULL, fdSocket, ZMQ_POLLIN, 0 };
		zmq::poll(&items[0], 1, SOCKET_POLL_TIMEOUT);

		//  If we get a message, print the contents
		if (items[0].revents & ZMQ_POLLIN)
		{
			std::string portStr;
			portStr.resize( 10 );

			int saSize = sizeof(struct sockaddr_in);
			size_t size = recvfrom( fdSocket, &portStr[0], 10, 0, (sockaddr*)&saListen, &saSize );
			
			portStr.resize( size );

			std::string ip(inet_ntoa(saListen.sin_addr));
			if (foundIps.find(ip) != foundIps.end())
				continue;

			foundIps.insert(ip);
			auto* node = co::newInstance("rpc.NetworkNode")->getService<rpc::INetworkNode>();
			node->setAddress(ip);	

			int port = std::stoi( portStr );
			node->setPort( port );
			instances.push_back(node);				
			
		}
		elapsed += zmq_stopwatch_stop( watch );

	} while( elapsed < timeout * 1000 );

	closesocket(fdSocket);

	return !foundIps.empty();
}
     
CORAL_EXPORT_COMPONENT( ZMQTransport, ZMQTransport );
    
} // namespace zmq
