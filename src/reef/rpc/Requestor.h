#ifndef _REEF_REQUESTOR_H_
#define _REEF_REQUESTOR_H_

#include "Marshaller.h"
#include "Demarshaller.h"

#include <co/Any.h>
#include <co/RefPtr.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

#include <map>

namespace reef {
namespace rpc {
    
class Node;
class ClientProxy;
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
};
    
class Requestor
{
public:
    Requestor( ClientRequestHandler* handler, const std::string& localEndpoint );
    
    ~Requestor();
    
    co::IObject* requestNewInstance( const std::string& componentName );
    
    co::IObject* requestPublicInstance( const std::string& key, const std::string& componentName );
    
    void requestAsynchCall( MemberOwner& owner, co::IMethod* method,  
                                 co::Range<co::Any const> args );
    
    void requestSynchCall( MemberOwner& owner, co::IMethod* method, 
                                co::Range<co::Any const> args, co::Any& ret );
    
    void requestSetField( MemberOwner& owner, co::IField* field, const co::Any arg );
    
    void requestGetField( MemberOwner& owner, co::IField* field, co::Any& ret );
    
    void requestLease( co::int32 instanceID );
    
    void requestLeaseBreak( co::int32 instanceID );
    
private:
    
    void onInterfaceParam( co::IService* param );
    
private:
        
    Marshaller _marshaller;
    Demarshaller _demarshaller;

    std::map<co::int32, ClientProxy*> _proxies;
    
    ClientRequestHandler* _handler;
    std::string _endpoint;
    std::string _localEndpoint;
};

}
}
#endif