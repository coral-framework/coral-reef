#ifndef __TESTTRANSPORT_TRANSPORT_H__
#define __TESTTRANSPORT_TRANSPORT_H__

#include "Transport_Base.h"

#include <reef/INode.h>
#include <reef/IActiveLink.h>
#include <reef/IPassiveLink.h>
#include <co/RefPtr.h>

namespace testTransport {
    
class Transport : public Transport_Base
{
public:
    Transport();
    
    virtual ~Transport();
    
    // ------ reef.ITransport Methods ------ //
    
    reef::IPassiveLink* bind( const std::string& addressToListen );
    
    reef::IActiveLink* connect( const std::string& addressToConnect );
    
    // ------ C++ only methods ------ //
    
    void post( const std::string& address, const std::string& msg );
    bool checkReply( const std::string& address, std::string& msg ); 
    
    bool check( const std::string& addressr, std::string& msg );
    void postReply( const std::string& address, const std::string& msg );
    
    void onLinkDestructor( const std::string& address );
protected:
    // ------ Receptacle 'node' (reef.INode) ------ //
    
    reef::INode* getNodeService();
    
    void setNodeService( reef::INode* node );
    
private:
    // member variables
    co::RefPtr<reef::INode> _nodeService;
};
    
} // namespace testTransport

#endif