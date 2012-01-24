/*
 * Component implementation template for 'reef.Node'.
 */
#include "Node.h"
#include "Networking.h"
#include "Proxy.h"
#include "Servant.h"
#include <reef/IProxy.h>
#include <co/IComponent.h>
#include <co/RefPtr.h>
#include <co/Coral.h>

namespace reef {

Node::Node()
{
	// empty constructor
}

Node::~Node()
{
	// empty destructor
}

// ------ reef.INode Methods ------ //

reef::IProxy* Node::createObject( co::int32 rmtNodeId, const std::string& componentName, co::int32 facetId )
{
    reef::Proxy* proxy = new reef::Proxy( this, rmtNodeId, _proxies.size() + 1 );
    _proxies.push_back( proxy );
    
    Message msg;
    msg.senderType = 2; //create object
    msg.data.resize( ( componentName.length() + 1 ) * sizeof( char ) );
    memcpy( &msg.data[0], static_cast<const void*>( componentName.c_str() ), ( componentName.size() + 1 ) * sizeof( char ) );
    msg.bytes = componentName.size() + 1;
    msg.senderId = facetId;

    Network::sendMsg( rmtNodeId, msg );
    
    return co::cast<reef::IProxy>( proxy->getService( "proxy" ) );
}

void Node::onProxyDestruction( co::int32 rmtNodeId, co::int32 proxyId )
{
    _proxies[proxyId] = _proxies.back();
    _proxies.pop_back();
    
    Message msg;
    //TODO setup msg to destroy remote Servant and Service as its corresponding proxy is being destroyed
    Network::sendMsg( rmtNodeId, msg );
}
    
void Node::processMsgs( co::int32 numMsgsToProccess )
{
    for( int i = 0; i < numMsgsToProccess; i++ )
    {
        co::int32 rmtNodeId;
        Message* msg;
        Network::getMessage( msg, rmtNodeId );
        processMsg( msg, rmtNodeId );
    }
}
    
void Node::processMsg( reef::Message *msg, co::int32 &rmtNodeId )
{
    if( msg->senderType == 0 ) // msg from Proxy to Servant
    {
        reef::Servant* servant = _servants[_proxyToServant.find( ProxyFullId( rmtNodeId, msg->senderId ) )->second];
        servant->receiveMsg( msg );
    }
    else if( msg->senderType == 1 ) // msg from Servant to Proxy
    {
        reef::Proxy* proxy = _proxies[msg->senderId];
        proxy->receiveMsg( msg );
    }
    else if( msg->senderType == 2 ) // msg from Node to Node
    {
        std::string objType( static_cast<const char*>( msg->data ) );
        co::IObject* masterObj = co::newInstance( objType );
        co::IComponent* masterType = masterObj->getComponent();
        co::IPort* masterFacet = masterType->getFacets()[msg->senderType]; //TODO: fix workaround
        co::IService* master = masterObj->getServiceAt( masterFacet );
        
        reef::Servant* servant = new reef::Servant( master, this, rmtNodeId, msg->senderId );
        _servants.push_back( servant );
        
        ProxyServantPair proxyServantPair( ProxyFullId( rmtNodeId, msg->senderId ), _servants.size() );
        _proxyToServant.insert( proxyServantPair );
    }
}
    
void Node::requestValueField( co::int32 rmtNodeId, co::int32 proxyId, co::int32 fieldId, co::Any& returnValue )
{
    Message msg;
    msg.data = static_cast<void*>( &fieldId );
    msg.type = 'n';
    msg.bytes = sizeof( int );
    msg.senderId = proxyId;
    msg.senderType = 0;
    
    Network::sendMsg( rmtNodeId, msg );
}

void Node::requestRefField( co::int32 rmtNodeId, co::int32 proxyId, co::int32 fieldId, co::IService*& returnValue )
{
    Message msg;
    msg.data = static_cast<void*>( &fieldId );
    msg.type = 'n';
    msg.bytes = sizeof( int );
    msg.senderId = proxyId;
    msg.senderType = 0;
    
    Network::sendMsg( rmtNodeId, msg );
}

void Node::callSynchMethod( co::int32 rmtNodeId, co::int32 proxyId, co::int32 methodId, co::Range<co::Any const> args, co::Any& returnValue )
{
    Message msg;
    msg.data = static_cast<void*>( &methodId );
    msg.type = 'n';
    msg.bytes = sizeof( int );
    msg.senderId = proxyId;
    msg.senderType = 0;
    
    Network::sendMsg( rmtNodeId, msg );
}

void Node::callAsynchMethod( co::int32 rmtNodeId, co::int32 proxyId, co::int32 methodId, co::Range<co::Any const> args )
{
    Message msg;
    msg.data = static_cast<void*>( &methodId );
    msg.type = 'n';
    msg.bytes = sizeof( int );
    msg.senderId = proxyId;
    msg.senderType = 0;
    
    Network::sendMsg( rmtNodeId, msg );
}
    
void Node::sendValueType( co::int32 rmtNodeId, co::int32 rmtProxyId, co::Any value )
{
}
    
CORAL_EXPORT_COMPONENT( Node, Node );
    
} // namespace reef
