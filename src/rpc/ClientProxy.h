#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "ClientProxy_Base.h"

#include "Demarshaller.h"
#include "Marshaller.h"

#include <rpc/IActiveLink.h>
#include <co/RefPtr.h>
#include <co/IService.h>
#include <co/RefVector.h>

namespace rpc {
    
class Requestor;
    
class ClientProxy : public ClientProxy_Base
{
public:
    ClientProxy( Requestor* requestor, co::IComponent* component, co::int32 instanceId );
    ClientProxy();
    virtual ~ClientProxy();
    
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
    
	co::int32 getInstanceId();
	Requestor* getRequestor();
    
    static inline bool isClientProxy( void* obj ) 
        { return *reinterpret_cast<void**>( obj ) == _classPtr; }
    
private:
    
    void setComponent( co::IComponent* component );
    
    /* Returns the depth in the hierarchy of \memberOwner among the supertypes of \facet. 
       Returns -1 in case memberOwner is the facet */
    co::int32 findDepth( co::IInterface* facet, co::ICompositeType* memberOwner );
    
private:
    co::RefPtr<Requestor> _requestor;
    
    static void* _classPtr;
    
    co::int32 _instanceID;
    
    Marshaller _marshaller;
    Demarshaller _demarshaller;
    
    co::Any _resultBuffer;
    
    co::int32 _numFacets;
    co::IService** _facets;
    co::IInterface** _interfaces;
    co::IComponent* _component;
};

} // namespace rpc

#endif