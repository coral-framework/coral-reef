#include <gtest/gtest.h>

#include <co/Coral.h>
#include <co/RefPtr.h>
#include <co/IObject.h>

#include <reef/ILocalServiceProxy.h>

#include <toto/IToto.h>

TEST( LocalObjectProxyTests, methodCalling )
{
	co::RefPtr<co::IObject> proxyObj = co::newInstance( "reef.LocalServiceProxy" );
	co::RefPtr<co::IObject> dummyObj = co::newInstance( "toto.Toto" );
	co::RefPtr<reef::ILocalServiceProxy> proxy = proxyObj->getService<reef::ILocalServiceProxy>();
	co::RefPtr<toto::IToto> dummy = dummyObj->getService<toto::IToto>();
	
	proxyObj->setService( "service", dummy.get() );

	char* msg = "sprintHello";
	
	
}