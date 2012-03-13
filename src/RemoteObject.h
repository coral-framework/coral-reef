#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"
#include "Channel.h"

namespace reef
{
    
class RemoteObject : public RemoteObject_Base
{
public:
    RemoteObject();
    RemoteObject( co::IComponent* component, Channel* channel );
    virtual ~RemoteObject();
    
    void setComponent( co::IComponent* component );
    
    // IComponent
    co::IComponent* getComponent();
    co::IService* getServiceAt( co::IPort* port );
    void setServiceAt( co::IPort* receptacle, co::IService* instance );
    
    // IDynamicServiceProvider
    co::IPort* dynamicGetFacet( co::int32 dynFacetId );
    const co::Any& dynamicGetField( co::int32 dynFacetId, co::IField* field );
    const co::Any& dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, 
                                 co::Range<co::Any const> args );
    co::int32 dynamicRegisterService( co::IService* dynamicServiceProxy );
    void dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value );

private:
    Channel* _channel;
    
    int _numFacets;
    co::Any _resultBuffer;
    co::IService** _facets;
    co::IComponent* _componentType;
};

} // namespace reef

#endif