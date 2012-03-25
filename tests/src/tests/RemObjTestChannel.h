#include "Encoder.h"

namespace reef
{

class RemObjTestChannel : public Encoder
{
public:
    RemObjTestChannel();

    ~RemObjTestChannel();
       
    // Creates a new instance and retrieves its unique id.
    virtual int newInstance( const std::string& typeName );
    virtual void sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args );
    
	// Only supports int32 double and string as return types
	virtual void call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result );
    virtual void getField( co::int32 serviceId, co::IField* field, co::Any& result );

    virtual void setField( co::int32 serviceId, co::IField* field, const co::Any& value );

    // ---------- Methods for testing only ------------ //
	virtual void getCalledValues( co::int32& serviceId, co::IMethod*& method, co::IField*& field )
	{
		serviceId = _lastServiceId;
		method = _lastMethod;
		field = _lastField;
	}

	virtual bool compareCalledValues( co::int32 serviceId, co::IMethod* method, co::IField* field )
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