#include <gtest/gtest.h>

#include <reef/IServerNode.h>

#include <co/Coral.h>
#include <co/IObject.h>
#include <Network/ConnectionServer.h>

TEST( ProxyTests, methodCalling )
{
    co::IObject* obj = co::newInstance( "reef.ServerNode" );
    reef::IServerNode* server = obj->getService<reef::IServerNode>();
    
    server->start( "tcp://*:5555" );
}