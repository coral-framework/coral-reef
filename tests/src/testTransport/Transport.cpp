#include "Transport.h"

namespace testTransport
{
    
Transport::Transport()
{
    // empty constructor
}

Transport::~Transport()
{
    // empty destructor
}

// ------ reef.ITransport Methods ------ //

reef::IPassiveLink* Transport::bind( const std::string& addressToListen )
{
    static co::RefPtr<reef::IPassiveLink> dummy;
    return dummy.get();
}

reef::IActiveLink* Transport::connect( const std::string& addressToConnect )
{
    static co::RefPtr<reef::IActiveLink> dummy;
    return dummy.get();
}

// ------ C++ only methods ------ //

void Transport::post( const std::string& address, const std::string& msg )
{
    
}
    
bool Transport::checkReply( const std::string& address, std::string& msg )
{
    return true;
}

bool Transport::check( const std::string& address, std::string& msg )
{
    return true;   
}

void Transport::postReply( const std::string& address, const std::string& msg )
{

}

void Transport::onLinkDestructor( const std::string& address )
{
    
}
// ------ Receptacle 'node' (reef.INode) ------ //

reef::INode* Transport::getNodeService()
{
    return _nodeService.get();
}

void Transport::setNodeService( reef::INode* node )
{
    _nodeService = node;
}

CORAL_EXPORT_COMPONENT( Transport, Transport );
    
} // namespace testTransport
