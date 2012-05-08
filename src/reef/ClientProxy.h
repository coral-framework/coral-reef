#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "ClientProxy_Base.h"

#include "Unmarshaller.h"
#include "Marshaller.h"

#include <reef/IActiveLink.h>
#include <co/RefPtr.h>
#include <co/IService.h>
#include <co/RefVector.h>

namespace reef
{
    
class Node;
    
class ClientProxy : public ClientProxy_Base
{
public:
    // Gets or creates a remote object pointing to an existing instance. 
    static ClientProxy* getOrCreateClientProxy( Node* node, co::IComponent* component, 
                                                 IActiveLink* link, co::int32 instanceID );
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
    
    // IInstanceInfo
	co::int32 getInstanceID();
	const std::string& getOwnerAddress();
    
    inline bool isLocalObject( void* obj ) { return *reinterpret_cast<void**>( obj ) != _classPtr; }
    
private:
    ClientProxy( Node* node, co::IComponent* component, IActiveLink* connecter, co::int32 instanceID );
    
    /* 
     Treats all possible cases of a TK_INTERFACE param in a call/field msg. 
     See Reference Values page on reef's wiki for further info. Also add the Ref param in the marshaller.
     */
    void onInterfaceParam( co::IService* param );
    
    void fetchReturnValue( co::IType* descriptor, co::Any& returnValue );
    
    void setComponent( co::IComponent* component );

    // Awaits a reply from a call. while waiting keeps updating Node.
    void awaitReplyUpdating( std::string& msg );
private:
    co::RefPtr<Node> _node;
    
    void* _classPtr;
    
    co::int32 _instanceID;
    
    Marshaller _marshaller;
    Unmarshaller _unmarshaller;
    co::RefPtr<IActiveLink> _link;
    co::Any _resultBuffer;    
    
    co::int32 _numFacets;
    co::IService** _facets;
    co::IComponent* _component;
};

} // namespace reef

#endif