#ifndef _RPC_REQUESTOR_H_
#define _RPC_REQUESTOR_H_

#include "Marshaller.h"
#include "Demarshaller.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>
#include <co/reserved/RefCounted.h>

#include <map>

namespace rpc {
    
class Node;
class ClientProxy;
class InstanceManager;
class RequestorManager;
class ClientRequestHandler;
   
/*!
 An absoulte reference to the owner of a member.
 
 This reference can identify within a host, which service and reflector should be used to invoke a
 member.
*/
struct MemberOwner
{
    co::int32 instanceID; //<! The id of the instance that provides the service
    co::int32 facetID; //<! The facet ID among the members of the component
    
    /*! The member owner Interface. -1 if the actual service interface is the owner. 
     0 for parent, 1 for a grandparent 2 for a greatgrandparent and so on... */         
    co::int32 inheritanceDepth;
    
    MemberOwner( co::int32 instanceID_, co::int32 facetID_, co::int32 inheritanceDepth_ )
    {
        instanceID = instanceID_;
        facetID = facetID_;
        inheritanceDepth = inheritanceDepth_;
    }
};
    
class Requestor : public co::RefCounted
{
public:
    Requestor( RequestorManager* manager, ClientRequestHandler* handler, 
              const std::string& publicEndpoint );
    
    // In case the node has been stopped, but the client proxies are still alive. If the requestor
    // is just deleted, then, a cancelLease call from a client proxy will result in an error.
    void disconnect();
    
    ~Requestor();
    
    // Sends a request for the creation of a new instance. Returns the proxy for it. Blocking.
    co::IObject* requestNewInstance( const std::string& componentName );
    
    // Sends a request for an instance publish under \param key. Returns the proxy for it. Blocking.
    co::IObject* requestPublicInstance( const std::string& key, const std::string& componentName );
    
    void requestAsynchCall( MemberOwner& owner, co::IMethod* method,  
                                 co::Slice<co::Any> args );
    
    void requestSynchCall( MemberOwner& owner, co::IMethod* method, 
                                co::Slice<co::Any> args, const co::Any& ret );
    
    void requestSetField( MemberOwner& owner, co::IField* field, const co::Any arg );
    
    void requestGetField( MemberOwner& owner, co::IField* field, const co::Any& ret );
    
    void requestLease( co::int32 instanceID, std::string lessee );
    
    void requestBarrierUp();
    
    void requestBarrierHit();
    
    void requestBarrierDown();
    
    ClientProxy* getOrCreateProxy( co::int32 instanceID, const std::string& componentName );
    
    inline const std::string& getEndpoint(){ return _endpoint; }
    
private:
    
    void pushParameters( co::IMethod* method, co::Slice<co::Any> args, 
                           ParameterPusher& pusher );
    
    // Extracts all the necessary info from \param param and saves it on \param refType.
    void getProviderInfo( co::IService* param, ReferenceType& refType );
    
    void getReturn( ParameterPuller& puller, co::IType* type, const co::Any& ret );

    void raiseReturnedException( Demarshaller& _demarshaller );
private:
        
    Marshaller _marshaller;
    Demarshaller _demarshaller;

    std::map<co::int32, ClientProxy*> _proxies;
    
    bool _connected;
    RequestorManager* _manager;
    ClientRequestHandler* _handler;
    InstanceManager* _instanceMan;
    std::string _endpoint;
    std::string _publicEndpoint;
    
    co::RefPtr<co::IObject> _tempRef; //TODO remove
};

} // namespace rpc
#endif