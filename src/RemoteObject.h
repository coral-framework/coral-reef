#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"
#include "Encoder.h"

#include <co/IService.h>
#include <co/RefVector.h>

namespace reef
{
    
class ServerNode;
    
class RemoteObject : public RemoteObject_Base
{
public:
    RemoteObject();
    RemoteObject( co::IComponent* component, Encoder* encoder, 
                 ServerNode* serverNode );
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
	/* 
		Check all the arguments for ref types. For each one found, it checks if the object is 
		local or remote and behaves accordingly. See reef's wiki for each case's explanation
		Under the 'Reference Arguments' page.
	*/
	void checkReferenceParams( co::IMethod* method, co::Range<co::Any const> args );

    void onLocalObjParam( co::IService* param );
    
    void onRemoteObjParam( co::IService* param );
    
	void onReferenceReturned( co::IMethod* method );
    
    inline bool isLocalObject( void* obj ) 
    { return *reinterpret_cast<void**>( obj ) != _remObjFingerprint; }
private:
    Encoder* _encoder;
    void* _remObjFingerprint;
    int _numFacets;
    ServerNode* _serverNode;
    co::Any _resultBuffer;
    co::IService** _facets;
    co::IComponent* _componentType;
};

} // namespace reef

#endif