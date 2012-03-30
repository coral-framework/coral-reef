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
    RemoteObject();
    RemoteObject( co::IComponent* component, const std::string& address );
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
    
    // IInstanceInfo
	co::int32 getInstanceID();
	const std::string& getOwnerAddress();
    
    inline bool isLocalObject( void* obj ) { return *reinterpret_cast<void**>( obj ) != _classPtr; }
    
private:
    /* 
     Treats all possible cases of a TK_INTERFACE param in a call/field msg. 
     See Reference Values page on reef's wiki for further info.
     */
    void onInterfaceParam( co::IService* param );
    
    void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );
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
    co::IComponent* _componentType;
};

} // namespace reef

#endif