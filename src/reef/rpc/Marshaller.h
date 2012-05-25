#ifndef __REEF_ENCODER_H__
#define __REEF_ENCODER_H__

#include <co/Any.h>
#include <co/Coral.h>

#include <string>

namespace reef {
namespace rpc {

    
class Message;
class Message_Member;
class Argument;

/*!
    Marshals remote requests and data into serialized strings.
 */
class Marshaller
{
    
public:
    
    /*!
        Represents the possible instance owner types from the requester's perspective. That is,
        A receiver will receive a LOCAL that mean actually an instance belonging to the owner, as 
        it is mapped through the sender's perspective.
     */
    enum RefOwner
    {
        LOCAL, //!< A local instance
        RECEIVER, //!< An instance in the Receiver
        ANOTHER //!< An instance in another host other than sender or receiver.
    };
    
    Marshaller();
    ~Marshaller();
    
    /*!
     Marshals a request for a new instance
     \param typeName to-be-instantiated component's type
     \param referer Request issuer ip (will be removed on future updates)
     \param request The marshalled request to be outputted by the method
     */
    void marshalNewInstance( const std::string& typeName, const std::string& referer, 
                            std::string& request );
    
    /*!
     Marshals a request for access to an instance, the instance's remote ref count sould be increased 
     or decreased and a referer added/removed based on \increment that tells if it is a request for
     starting of stopping access to the instance.
     
     The start access type of request is tricky and need detailing:
     The case is when A pass a reference parameter R to B, and that reference is to an object in C.
     Therefore, C needs to increment R's refcounting before A sends R to B, else there could be an
     inconsistent state if A removed its reference to R before B got the chance to increase it, C
     would delete the object and B would get an invalid reference. 
     Moreover, this request is always issued by A to C and not by B to C, as it should intuitively be.
     However, B's ip is the one passed as \referer.
     i.e: A would create an access request filling B's address as \referer and send it to C.
     
     The stop access request is intuitive, the proxy holder is the one that makes the call to host.
     \param instanceId Id of the instance interface inside the host node.
     \param increment Tells if this is a request for starting of stopping access to an instance.
     \param referer The node that is responsible for the access (B in the above explanation)
     \param request The marshalled request that is outputted by the method.
    */
    void marshalAccessInstance( co::int32 instanceId, bool increment, const std::string& referer, 
                               std::string& request );
    
    /*! 
     Marshals a request for finding an existing instance that has been published through a key.
     \param key the string that the host has published the instance under.
     \param referer The address of the node that is requesting access to the published instance.
     \param request The marshalled request that is outputted by the method.
    */
    void marshalFindInstance( const std::string& key, const std::string& referer, 
                             std::string& request );
    
    /*!
     Marshaller functions for Call/Field requests. As these requests require different amounts of 
     parameters of different types, they can't be marshalled with a single call. Therefore, the
     begin method starts an internal state of request marshalling, then, after every argument is added,
     the getMarshalledCall method should be called to get the ready request, also, the state is reset.

     Starts an internal state of call request marshalling, must be matched with a getMarshalledCall.
     \param instanceId is the destination object whose facet's method is being called
     \param facetIdx is the index of the facet within the object's members
     \param memberIdx is the index of the called member within the facet's members
     \param typeDepth is the depth of the member owner into the facet's hierarchy, starting at -1 if the
     facets type is the memberowner and increasing for each level above in the inheritance tree.
     \param hasReturn is necessary to know if we are dealing 
     \param callerAddress is necessary until there is a way to retrieve the address of a msg sender.
    */
    void beginCallMarshalling( co::int32 instanceId, co::int32 facetIdx, co::int32 memberIdx,
                            co::int32 typeDepth, bool hasReturn, const std::string& callerAddress );
    
    //! adds a Value Type parameter
    void addValueParam( const co::Any& param );
    
    /*!
     adds a ref-type parameter.
     \param instanceId Id of the instance providing the parameter's interface inside the host
     \param facetIdx Id of the interface of the parameter within the provider`s members
     \param owner Type of the owner. See RefOwner Type.
     \param instanceType The instance's component type. not necessary if owner is RECEIVER
     \param ownerAddress The instance owner's address. not necessary if owner is RECEIVER (LOCAL also in 
     future)
    */
    void addReferenceParam( co::int32 instanceId, co::int32 facetIdx, RefOwner owner, 
                     const std::string* instanceType = 0, const std::string* ownerAddress = 0 );
    
    //! gets the marshalled call request with all the arguments
    void getMarshalledCall( std::string& request );
    
    
    /*!
     Marshals a rogue value type. Works the same way of addReferenceParam, but returns the 
     marshalled value instead of adding it as a parameter to a call request.
    */
    void marshalValueType( const co::Any& demarshalledValue, std::string& marshalledValue );
    
    /*!
     Marshals a rogue reference type. Works the same way of addReferenceParam, but returns the 
     marshalled reference instead of adding it as a parameter to a call request.
     */
    void marshalReferenceType( co::int32 instanceId, co::int32 facetIdx, RefOwner owner, 
                              std::string& reference, const std::string* instanceType = 0, 
                              const std::string* ownerAddress = 0 ); 
    
    //! ----- Marshals plain data messages ----- //
    void marshalData( bool value, std::string& data );
    
    void marshalData( double value, std::string& data );
    
    void marshalData( co::int32 value, std::string& data );
    
    void marshalData( const std::string& value, std::string& data );
    
private:
    void reference2PBArg( co::int32 instanceId, co::int32 facetIdx, RefOwner owner, 
                         Argument* PBArg, const std::string* instanceType = 0, 
                         const std::string* ownerAddress = 0 );
    
    void checkIfCallMsg();
    
private:
    Message* _message;
    Message_Member* _msgMember;
    
};
    
}
    
}
#endif