#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"
#include <reef/INode.h>

namespace reef
{
    
class RemoteObject : public RemoteObject_Base
{
public:
    RemoteObject();
    virtual ~RemoteObject();
    
    void setComponent( co::IComponent* component );
    
    co::IComponent* getComponent();
    co::IService* getServiceAt( co::IPort* port );
    void setServiceAt( co::IPort* receptacle, co::IService* instance );
    co::IPort* dynamicGetFacet( co::int32 dynFacetId );
    const co::Any& dynamicGetField( co::int32 dynFacetId, co::IField* field );
    const co::Any& dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, co::Range<co::Any const> args );
    co::int32 dynamicRegisterService( co::IService* dynamicServiceProxy );
    void dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value );

private:
    INode* _owner;
    int _numFacets;
    co::IService** _facets;
    co::IComponent* _componentType;
};

} // namespace reef

#endif