#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"
#include "Encoder.h"

#include <co/IService.h>
#include <co/RefVector.h>

namespace reef
{
    
class IServerNode;
    
class RemoteObject : public RemoteObject_Base
{
public:
    RemoteObject();
    RemoteObject( co::IComponent* component, Encoder* encoder );
    virtual ~RemoteObject();
    
    void setComponent( co::IComponent* component );
    
    inline void setServerNode( IServerNode* serverNode ) 
        { _serverNode = serverNode; }
    
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
    
    // These functions are used for differentiating a RemoteObject from an Object
	static void* getClassPtr();
    static inline bool isLocalObject( void* obj )
    { return *reinterpret_cast<void**>( obj ) != RemoteObject::getClassPtr(); }
    
private:
    static void* _classPtr;
    
    Encoder* _encoder;
    int _numFacets;
    IServerNode* _serverNode;
    co::Any _resultBuffer;
    co::IService** _facets;
    co::IComponent* _componentType;
};

} // namespace reef

#endif