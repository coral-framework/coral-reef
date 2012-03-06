#include "Channel.h"

namespace reef
{
//! Template abstract class.
class RemObjTestChannel : public Channel
{
public:
    RemObjTestChannel();

    ~RemObjTestChannel();
       
    // Creates a new instance and retrieves its unique id.
    int newInstance( const std::string& typeName );
    void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    
	// Only supports int32 double and string as return types
	void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    void getField( co::int32 serviceId, co::IField* field, co::Any& result );

    void setField( co::int32 serviceId, co::IField* field, const co::Any& value );

    // Writes a raw event into channel.
    void write( const Message* message );

	// ---------- Methods for testing only ------------ //

	void getCalledValues( co::int32& serviceId, co::IMethod*& method, co::IField*& field )
	{
		serviceId = _lastServiceId;
		method = _lastMethod;
		field = _lastField;
	}

	bool compareCalledValues( co::int32 serviceId, co::IMethod* method, co::IField* field )
	{
		return serviceId == _lastServiceId && method == _lastMethod && field == _lastField;
	}

private:
	co::int32 _lastServiceId;
	co::IMethod* _lastMethod;
	co::IField* _lastField;
    std::string _dummyStr;

	void setLastValues( co::int32 serviceId, co::IMethod* method, co::IField* field )
	{
		_lastServiceId = serviceId;
		_lastMethod = method;
		_lastField = field;
	}

	//helper function
	void setReturnValue( co::IType* descriptor, co::Any& retValue );

};

}