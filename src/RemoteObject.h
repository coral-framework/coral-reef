#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"

#include "Decoder.h"
#include "Encoder.h"
#include "network/Connection.h"

#include <co/IService.h>
#include <co/RefVector.h>

namespace reef
{
    
class IServerNode;
    
class RemoteObject : public RemoteObject_Base
{
public:
    // Gets or creates a remote object pointing to an existing instance. 
    static RemoteObject* getOrCreateRemoteObject( co::IComponent* component, Connecter* connecter,
                                             co::int32 instanceID = -1 );

    virtual ~RemoteObject();
    
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
    
    // IInstanceInfo
	co::int32 getInstanceID();
	const std::string& getOwnerAddress();
    
    inline bool isLocalObject( void* obj ) { return *reinterpret_cast<void**>( obj ) != _classPtr; }
    
private:
    RemoteObject();
    RemoteObject( co::IComponent* component, Connecter* connecter, co::int32 instanceID );
    
    /* 
     Treats all possible cases of a TK_INTERFACE param in a call/field msg. 
     See Reference Values page on reef's wiki for further info.
     */
    void onInterfaceParam( co::IService* param );
    
    void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );
    
    void setComponent( co::IComponent* component );

private:
    void* _classPtr;
    
    co::int32 _instanceID;
    
    Encoder _encoder;
    Decoder _decoder;
    co::RefPtr<Connecter> _connecter;
    co::Any _resultBuffer;
    IServerNode* _serverNode;
    
    
    co::int32 _numFacets;
    co::IService** _facets;
    co::IComponent* _component;
};

} // namespace reef

#endif