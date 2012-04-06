#include "RemoteObject.h"

#include "network/Connection.h"

#include <reef/IServerNode.h>

#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IReflector.h>
#include <co/IParameter.h>

#include <map>

namespace reef {

class Host
{
public:
    inline void addInstance( RemoteObject* remoteObj, co::int32 instanceID )
    {
        _instances.insert( std::pair<co::int32, RemoteObject*>( instanceID, remoteObj ) );
    }
    
    inline RemoteObject* getInstance( co::int32 instanceID )
    {
        InstanceMap::iterator it = _instances.find( instanceID );
        return it != _instances.end() ? it->second : 0;
    }
    
    inline void removeInstance( co::int32 instanceID )
    {
        InstanceMap::iterator it = _instances.find( instanceID );
        _instances.erase( it );
    }
private:
    typedef std::map<co::int32, RemoteObject*> InstanceMap;
    InstanceMap _instances;
};

typedef std::map<Connecter*, Host*> Hosts;
static Hosts _hosts;
    
RemoteObject* RemoteObject::getOrCreateRemoteObject( co::IComponent* component,
                                            Connecter* connecter, co::int32 instanceID )
{
    Host* host = 0;
    RemoteObject* retValue = 0;
    Hosts::iterator it = _hosts.find( connecter );
    if( it != _hosts.end() )
    {
        host = it->second;
        retValue = host->getInstance( instanceID );
        
        if( retValue )
            return retValue;
    }
    else
    {
        host = new Host();
        _hosts.insert( std::pair<Connecter*, Host*>( connecter, host ) );
    }
    
    retValue = new RemoteObject( component, connecter, instanceID );
    
    return retValue;
}
    
RemoteObject::RemoteObject()
{
}
    
RemoteObject::RemoteObject( co::IComponent* component, Connecter* connecter, co::int32 instanceID ) :
    _connecter( connecter ), _numFacets( 0 )
{
    _classPtr = *reinterpret_cast<void**>( this );
    setComponent( component );
    _instanceID = instanceID;
}
    
RemoteObject::~RemoteObject()
{    
    for( int i = 0; i < _numFacets; i++ )
    {
        delete _facets[i];
    }
    delete [] _facets;
}
    
void RemoteObject::setComponent( co::IComponent* component )
{    
	_component = component;
    
	// create proxy interfaces for our facets
	co::Range<co::IPort* const> facets = _component->getFacets();
	int numFacets = static_cast<int>( facets.getSize() );
    _facets = new co::IService*[numFacets];
	for( int i = 0; i < numFacets; ++i )
	{
		facets[i]->getType()->getReflector()->newDynamicProxy( this );
	}
}

co::IComponent* RemoteObject::getComponent()
{
    return _component;
}

co::IService* RemoteObject::getServiceAt( co::IPort* port )
{
    co::ICompositeType* owner = port->getOwner();
    if( owner == _component )
    {
        int portIndex = port->getIndex();
        assert( portIndex <= _numFacets );
        return _facets[portIndex];
    }
    
    return reef::RemoteObject_Base::getServiceAt( port );
}

void RemoteObject::setServiceAt( co::IPort* receptacle, co::IService* instance )
{
    // empty: setting remote services not supported yet
}

co::IPort* RemoteObject::dynamicGetFacet( co::int32 dynFacetId )
{
    co::IService* service = _facets[dynFacetId];
    return service->getFacet();
}
        
const co::Any& RemoteObject::dynamicGetField( co::int32 dynFacetId, co::IField* field )
{
    _encoder.beginEncodingCallMsg( _instanceID, dynFacetId, field->getIndex(), true );
    std::string msg;
    _encoder.finishEncodingCallMsg( msg );
    _connecter->send( msg );
    
    _connecter->receiveReply( msg );
    _decoder.decodeData( msg, field->getType(), _resultBuffer );

    return _resultBuffer;
}
    
void RemoteObject::dynamicSetField( co::int32 dynFacetId, co::IField* field, const co::Any& value )
{
    _encoder.beginEncodingCallMsg( _instanceID, dynFacetId, field->getIndex(), true );
    std::string msg;
    _encoder.finishEncodingCallMsg( msg );
    _connecter->send( msg );

}

const co::Any& RemoteObject::dynamicInvoke( co::int32 dynFacetId, co::IMethod* method, 
                                           co::Range<co::Any const> args )
{
    co::IType* returnType = method->getReturnType();
    if( returnType )
        _encoder.beginEncodingCallMsg( _instanceID, dynFacetId, method->getIndex(), true );
    else
        _encoder.beginEncodingCallMsg( _instanceID, dynFacetId, method->getIndex(), false );
    
    for( ; args; args.popFirst() )
    {
        const co::Any& arg = args.getFirst();
        
        if( arg.getKind() != co::TK_INTERFACE )
            _encoder.addValueParam( arg );
        else
            onInterfaceParam( arg.get<co::IService*>() );
    }
    
    std::string msg;
    _encoder.finishEncodingCallMsg( msg );
    _connecter->send( msg );
    
	if( returnType )
    {
        _connecter->receiveReply( msg );
        _decoder.decodeData( msg, method->getReturnType(), _resultBuffer );
    }

    return _resultBuffer;
}

co::int32 RemoteObject::dynamicRegisterService( co::IService* dynamicServiceProxy )
{
    _facets[_numFacets] = dynamicServiceProxy;
	return _numFacets++;
}
     
void RemoteObject::onInterfaceParam( co::IService* param )
{
    co::IObject* provider = param->getProvider();
    co::int32 facetIdx = param->getFacet()->getIndex();
    co::int32 instanceID;
    
    if( isLocalObject( provider ) )
    {
        instanceID = _serverNode->publishInstance( provider );
        _encoder.addRefParam( instanceID, facetIdx, Encoder::RefOwner::LOCAL );
    }
    else // is a remote object, so it provides the IInstanceInfo service
    {
        IInstanceInfo* info = provider->getService<IInstanceInfo>();
        
        instanceID = info->getInstanceID();
        const std::string& ownerAddress = info->getOwnerAddress();
        
        if( ownerAddress == _connecter->getAddress() ) // Receiver
        {
            _encoder.addRefParam( instanceID, facetIdx, Encoder::RefOwner::RECEIVER );
        }
        else
        {
            _encoder.addRefParam( instanceID, facetIdx, Encoder::RefOwner::ANOTHER, &ownerAddress );
        }
    }
}

    // ------ reef.IInstanceInfo Methods ------ //
    
co::int32 RemoteObject::getInstanceID()
{
    return _instanceID;
}

const std::string& RemoteObject::getOwnerAddress()
{
    return _connecter->getAddress();
}

CORAL_EXPORT_COMPONENT( RemoteObject, RemoteObject );
    
} // namespace reef
