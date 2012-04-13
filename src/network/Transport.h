#ifndef _REEF_TRANSPORT_H_
#define _REEF_TRANSPORT_H_

#include <co/reserved/RefCounted.h>

#include <map>

namespace reef
{
   
class Connecter;
class Binder;
    
/*
    Kind of simpler abstract factory. Creates and manages resources for the specific transport layer
    implementations. Usage: get a concrete factory by using the factory method getConcreteTransport
    and use its methods.
 */
class Transport
{
public:
    enum Transports
    {
        ZMQ = 0,
        LIST = 1
    };
    
    virtual ~Transport();
    
    // Set the concrete transport implementation.
    static void setConcreteTransport( Transports concreteType );
    
    // Get the concrete Transport factory. Shared resource also, usage of refPtrs recommended.
    static Transport* getInstance();
    static void clearInstance();
    
    // Delegates the actual Connecter creation to the concrete Transports. Maps the Connecter to the
    // address so it can be shared.
    Connecter* getConnecter( const std::string& address );
    
    // Caveat: Connecters are shared resources, need to be deleted before the mapping is cleared.
    // Method preferably called from within the Connecter destructor.
    void clearConnecter( const std::string& address );
    
    // Delegates the creation of a binder to the concrete transport. Binders dont need to be shared
    Binder* newBinder( const std::string& address );
    
protected:
    // to be implemented by the actual Transport implementation
    virtual Connecter* createConnecter( const std::string& addressToConnect ) = 0;
    virtual Binder* createBinder( const std::string& addressBoundTo ) = 0;
    
private:
    static Transport* _instance;
    
    typedef std::map<std::string, Connecter*> Connecters;
    typedef std::map<std::string, Binder*> Binders;
    // map holding all open connections
    Connecters _connecters;
    Binders _binders;

};
    
class Connecter : public co::RefCounted
{
public:
    	virtual void send( const std::string& data ) = 0;
	virtual bool receiveReply( std::string& data ) = 0; // non-blocking

    virtual const std::string& getAddress() = 0;
};

 
class Binder : public co::RefCounted
{
public:
    virtual bool receive( std::string& data ) = 0; // Non-Blocking
	virtual void reply( const std::string& data ) = 0;

    virtual const std::string& getAddress() = 0;
};

} // namespace reef

#endif