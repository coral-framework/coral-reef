#include "Toto_Base.h"

#include <iostream>
#include <stdio.h>

static void printMethodCount( int& count )
{
    std::cout << "Method call number '" << count++ << "'" << std::endl;
}

namespace toto{

class Toto : public Toto_Base
{
public:

	Toto()
	{
        _count = 0;
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
        printMethodCount( _count );
		std::cerr << "HELLO WORLD!" << std::endl;
    }
    
    void printWelcome()
    {
        printMethodCount( _count );
        std::cerr << "WELCOME!" << std::endl;
    }
	
	void printMethod3()
    {
        printMethodCount( _count );
        std::cerr << "THIS IS METHOD3!" << std::endl;
    }
	
private:
    int _count;
	co::int32 _number;
	
};
CORAL_EXPORT_COMPONENT( Toto, Toto );
};