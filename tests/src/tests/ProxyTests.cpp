#include <gtest/gtest.h>

#include <RemoteObject.h>
#include <co/IObject.h>
#include <co/RefPtr.h>
#include <toto/IToto.h>

TEST( ProxyTests, methodCalling )
{
    co::RefPtr<reef::RemoteObject> totoObj( new reef::RemoteObject() );
    co::IComponent* componentType = co::cast<co::IComponent>( co::getType( "toto.Toto" ) );
    totoObj->setComponent( componentType );
    toto::IToto* toto = totoObj->getService<toto::IToto>();
    toto->printHello();
}