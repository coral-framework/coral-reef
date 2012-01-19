#include <gtest/gtest.h>

#include <reef/INode.h>
#include <reef/IProxy.h>

#include <co/RefPtr.h>
#include <Proxy.h>
#include <Networking.h>
#include <toto/IToto.h>

TEST( ProxyTests, methodCalling )
{
    co::RefPtr<co::IObject> nodeObj = co::newInstance( "reef.Node" );
    reef::INode* node = nodeObj->getService<reef::INode>();
    
    co::RefPtr<reef::IProxy> proxy = node->createObject( 0, "toto.Toto", 0 );
    proxy->callMethodById( 2 );
}