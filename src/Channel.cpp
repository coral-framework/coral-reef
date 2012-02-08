#include "Channel.h"
#include "Event.pb.h"

#include <iostream>

namespace reef 
{
    
static void printEvent( Event* event )
{
    std::cout << "EVENT INFO:" << std::endl;
    std::cout << "Destination:" << event->destination() << std::endl;
    
    std::cout << event->eventtype() << std::endl;
}
    
void Channel::route( const std::string& data, const std::vector<Channel*>& channels )
{
    Event event;
    event.ParseFromString( data );
    
    printEvent( &event );

    int dest = event.destination();
    assert( dest >= 0 && dest < channels.size() );
    
    channels[dest]->write( &event );
}
    
Channel::Channel( Connection* connection ) : _channelId( -1 ), _connection( connection )
{
    // empty
}

Channel::~Channel()
{
    // empty
}

// InputChannel
    InputChannel::InputChannel( Connection* connection ) : Channel( connection )
{   
    _connection = connection;
}
    
InputChannel::~InputChannel()
{
    // empty
}            

int InputChannel::newInstance( const std::string& typeName )
{
    assert( _channelId == - 1 );
    
    Event event;
    event.set_destination( 0 ); // 0 is always the node channel
    event.set_eventtype( Event::TYPE_CREATE );
    
	EventCreate* ce = event.mutable_eventcreate();
    ce->set_componenttypename( typeName );
    
    write( &event );
    
    std::string input;
    _connection->receive( input );
    
    event.ParseFromString( input );
    assert( event.eventtype() == Event::TYPE_CREATE_RESULT );
 
    _channelId = event.resultcreate().virtualaddress();
    
    return _channelId;
}
    
static EventCall* makeEventCall( bool hasReturn, Event& owner, co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    EventCall* ce = owner.mutable_eventcall();
    ce->set_hasreturn( hasReturn );
    ce->set_serviceindex( serviceId );
    ce->set_methodindex( methodIndex );

    // TODO: serialize parameters
    return ce;
}
                                
void InputChannel::sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    Event event;
    event.set_destination( _channelId ); // 0 is always the node channel
    event.set_eventtype( Event::TYPE_CALL );
    
    // make a call without returns
    makeEventCall( false, event, serviceId, methodIndex, args );
    write( &event );
}

void InputChannel::call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result )
{
    Event event;
    event.set_destination( _channelId ); // 0 is always the node channel
    event.set_eventtype( Event::TYPE_CALL );
    
    // signalize that a return is expected
    makeEventCall( true, event, serviceId, methodIndex, args );
    
    write( &event );
    
    // wait for the return
    std::string input;
    _connection->receive( input );
    
    event.ParseFromString( input );
    assert( event.eventtype() == Event::TYPE_CALL_RETURN );
    
    // TODO: set returned value into result variable
}

void InputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    Event event;
    event.set_destination( _channelId ); // 0 is always the node channel
    event.set_eventtype( Event::TYPE_FIELD );
    
    EventField* fe = event.mutable_eventfield();
    fe->set_issetfield( false ); // it is a get field event
    fe->set_serviceindex( serviceId );
    fe->set_fieldindex( fieldIndex );
    
    write( &event );
    
    // wait for the field value
    std::string input;
    _connection->receive( input );
    
    event.ParseFromString( input );
    assert( event.eventtype() == Event::TYPE_FIELD );
    
    // TODO: set the returned field value into result variable
}

void InputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    Event event;
    event.set_destination( _channelId ); // 0 is always the node channel
    event.set_eventtype( Event::TYPE_FIELD );
    
    EventField* fe = event.mutable_eventfield();
    fe->set_issetfield( true ); // it is a set field event
    fe->set_serviceindex( serviceId );
    fe->set_fieldindex( fieldIndex );
    
    write( &event );
}
    
void InputChannel::write( const reef::Event* event )
{
    std::string output;
    event->SerializeToString( &output );
    
    _connection->send( output );
}
    
// OutputChannel
OutputChannel::OutputChannel( Connection* connection, OutputChannelDelegate* delegate )
    : Channel( connection ) 
{
    _delegate = delegate;
}
    
OutputChannel::~OutputChannel()
{
    // empty
}

int OutputChannel::newInstance( const std::string& typeName )
{
    return _delegate->onNewInstance( this, typeName );
}

void OutputChannel::sendCall( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args )
{
    _delegate->onSendCall( this, serviceId, methodIndex, args );
}
    
void OutputChannel::call( co::int32 serviceId, co::int32 methodIndex, co::Range<co::Any const> args, co::Any& result )
{
    _delegate->onCall( this, serviceId, methodIndex, args, result );
}
    
void OutputChannel::getField( co::int32 serviceId, co::int32 fieldIndex, co::Any& result )
{
    _delegate->onGetField( this, serviceId, fieldIndex, result );
}
    
void OutputChannel::setField( co::int32 serviceId, co::int32 fieldIndex, const co::Any& value )
{
    _delegate->onSetField( this, serviceId, fieldIndex, value );
}
    
void OutputChannel::write( const reef::Event* event )
{    
    co::Range<co::Any const> dummy;

    Event::EventType type = static_cast<Event::EventType>( event->eventtype() );
    
    switch ( type ) {
        case Event::TYPE_CREATE:
        {
            const EventCreate& EventCreate = event->eventcreate();
            int virtualAddress = newInstance( EventCreate.componenttypename() );

            Event resultEvent;
            resultEvent.set_eventtype( Event::TYPE_CREATE_RESULT );
            resultEvent.set_destination( virtualAddress ); // destination makes no much sense here since is 
                                                           // answer is always deterministic
            
            // send back the recently created virtual address
            ResultCreate* cr = resultEvent.mutable_resultcreate();
            cr->set_virtualaddress( virtualAddress );
            
            std::string output;
            resultEvent.SerializeToString( &output );
            _connection->send( output );
            
            break;
        } 
        case Event::TYPE_CALL:
        {
            const EventCall& EventCall = event->eventcall();
            co::int32 serviceId = EventCall.serviceindex();
            co::int32 methodIndex = EventCall.methodindex();
            
            // TODO: handle call arguments (translate to co::Any)
            // const DataArgument& argument = call.arguments( 0 );
            
            if( EventCall.hasreturn() )
            {
                co::Any result;
                call( serviceId, methodIndex, dummy, result );
                
                // TODO: handle return
            }
            else
            {
                sendCall( serviceId, methodIndex, dummy );
            }
            break;
        }
        default:
            break;
    }
}

} // namespace reef