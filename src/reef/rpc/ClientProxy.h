#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "ClientProxy_Base.h"

#include "Demarshaller.h"
#include "Marshaller.h"

#include <reef/rpc/IActiveLink.h>
#include <co/RefPtr.h>
#include <co/IService.h>
#include <co/RefVector.h>

namespace reef {
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
	const std::string& getOwnerAddress();
    
    static inline bool isLocalObject( void* obj ) 
        { return *reinterpret_cast<void**>( obj ) != s_classPtr; }
    
private:
    
    
    /* 
     Treats all possible cases of a TK_INTERFACE param in a call/field msg. 
     See Reference Values page on reef's wiki for further info. Also add the Ref param in the marshaller.
     */
    void onInterfaceParam( co::IService* param );
    
    void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );
    
    void setComponent( co::IComponent* component );

    void demarshalReturn( const std::string& data, co::IType* returnedType, 
                                      co::Any& returned );
    // Awaits a reply from a call. while waiting keeps updating Node.
    void awaitReplyUpdating( std::string& msg );
    
    /* Returns the depth in the hierarchy of \memberOwner among the supertypes of \facet. 
       Returns -1 in case memberOwner is the facet */
    co::int32 findDepth( co::IInterface* facet, co::ICompositeType* memberOwner );
private:
    co::RefPtr<Requestor> _requestor;
    
    static void* s_classPtr;
    
    co::int32 _instanceID;
    
    Marshaller _marshaller;
    Demarshaller _demarshaller;
    
    co::Any _resultBuffer;
    co::RefPtr<co::IObject> _tempRef; //TODO remove
    
    co::int32 _numFacets;
    co::IService** _facets;
    co::IInterface** _interfaces;
    co::IComponent* _component;
};

}
    
} // namespace reef

#endif