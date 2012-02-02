#include "Toto_Base.h"

#include <iostream>
#include <stdio.h>

namespace toto{

class Toto : public Toto_Base
{
public:

	Toto()
	{
	}
	
	virtual ~Toto()
	{
	}
	
	co::int32 getNumber()
	{
		return _number;
	}
	
	void setNumber( co::int32 number )
	{
		_number = number;
	}
	
	co::int32 numberGet()
	{
		return _number;
	}
	
	void printHello()
	{
		std::cerr << "HELLO WORLD!";
    }
	
private:

	co::int32 _number;
	
};
CORAL_EXPORT_COMPONENT( Toto, Toto );
};