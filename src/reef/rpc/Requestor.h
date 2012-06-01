#ifndef _REEF_REQUESTOR_H_
#define _REEF_REQUESTOR_H_

#include "Marshaller.h"
#include "Demarshaller.h"

#include <co/Any.h>
#include <co/IObject.h>
#include <co/reserved/RefCounted.h>

#include <map>

namespace reef {
namespace rpc {
    
class Node;
class ITransport;
class IActiveLink;
class ClientProxy;
class RequestorCache;
   
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
    };
    
class Requestor : public co::RefCounted
{
public:
    Requestor( Node* node, IActiveLink* link, const std::string& localEndpoint, 
              RequestorCache* manager );
    
    ~Requestor();
    
    co::IObject* requestNewInstance( const std::string& componentName );
    
    co::IObject* requestPublicInstance( const std::string& key, const std::string& componentName );
    
    void requestAsynchInvocation( MemberOwner& owner, co::IMethod* method,  co::Range<co::Any const> args );
    
    void requestAsynchInvocation( co::int32 dynFacetId,co::int32 inheritanceDepth,
                                 co::IField* field, const co::Any arg );
    
    void requestSynchInvocation( co::int32 dynFacetId, co::int32 inheritanceDepth,
                                co::IMethod* method, co::Range<co::Any const> args, co::Any& ret );
    
    void requestSynchInvocation( co::int32 dynFacetId, co::int32 inheritanceDepth, co::IField* field,
                                co::Any& ret );
    
    void requestLease( co::int32 instanceID );
    
    void requestLeaseBreak( co::int32 instanceID );
    
    inline const std::string& getEndpoint(){ return _endpoint; }
    
private:
    Node* _node;
    IActiveLink* _link;
    std::string _endpoint;
    std::string _localEndpoint;
    RequestorCache* _manager;
    
    std::map<co::int32, ClientProxy*> _proxies;
    
    Marshaller _marshaller;
    Demarshaller _demarshaller;
};

}
}
#endif