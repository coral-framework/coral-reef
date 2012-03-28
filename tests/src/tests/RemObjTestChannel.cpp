#include "RemObjTestChannel.h"

#include <co/IField.h>
#include <co/IMethod.h>

#include <co/IllegalCastException.h>

namespace reef
{

	RemObjTestChannel::RemObjTestChannel() : Encoder( 0, 0 ), _lastServiceId( -1 ), _lastMethod( 0 ),
		_lastField( 0 )
	{
	}

    RemObjTestChannel::~RemObjTestChannel()
	{
	}
       
    // Creates a new instance and retrieves its unique id.
    int RemObjTestChannel::newInstance( const std::string& typeName )
	{
		return 0;
	}
    
    void RemObjTestChannel::sendCall( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args )
	{
		setLastValues( serviceId, method, 0 );
	}

    void RemObjTestChannel::call( co::int32 serviceId, co::IMethod* method, co::Range<co::Any const> args, co::Any& result )
	{
		// code for the return type to work
		co::IType* retType = method->getReturnType();
		setReturnValue( retType, result );
		
		setLastValues( serviceId, method, 0 );
	}

    void RemObjTestChannel::getField( co::int32 serviceId, co::IField* field, co::Any& result )
	{
		// code for the return type to work
		co::IType* retType = field->getType();
		setReturnValue( retType, result );
		setLastValues( serviceId, 0, field );
	}

    void RemObjTestChannel::setField( co::int32 serviceId, co::IField* field, const co::Any& value )
	{
		setLastValues( serviceId, 0, field );
	}

	void RemObjTestChannel::setReturnValue( co::IType* descriptor, co::Any& retValue )
	{
		co::TypeKind kind = descriptor->getKind();

		if( kind == co::TK_INT32 )
			retValue.set<co::int32>( 1 );
		else if( kind == co::TK_DOUBLE )
			retValue.set<double>( 1.0 );
		else if( kind == co::TK_STRING )
			retValue.set<std::string&>( _dummyStr );
		else
			throw new co::IllegalCastException( "Fakechannel supports only int double and string as ret values" );
	}

};