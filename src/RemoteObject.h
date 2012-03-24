#ifndef __REMOTEOBJECT_H__
#define __REMOTEOBJECT_H__

#include "RemoteObject_Base.h"
#include "Channel.h"

#include <co/IService.h>
#include <co/RefVector.h>

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
	/* 
		Check all the arguments for ref types. For each one found, it checks if the object is 
		local or remote and behaves accordingly. See reef's wiki for each case's explanation
		Under the 'Reference Arguments' page.
	*/
	void checkReferenceParams( co::IMethod* method, co::Range<co::Any const> args );

	void RemoteObject::onReferenceReturned( co::IMethod* method );
private:
    Channel* _channel;
    
    int _numFacets;
    co::Any _resultBuffer;
    std::vector<co::IService*> _facets;
    co::IComponent* _componentType;
};

} // namespace reef

#endif