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
	
	void printInt( co::int32 number )
	{
		std::cerr << "Printing int: " << number << std::endl;
	}

	void printString( const std::string& str )
	{
		std::cerr << "Printing string: " << str << std::endl;
	}

	void printDouble( double number )
	{
		std::cerr << "Printing double: " << number << std::endl;
	}
    
    void printStringList( co::Range<std::string const> strList )
    {
        for( int i = 0; strList; ++i, strList.popFirst() )
        {
            std::cout << "PRINT STRING ELEMENT : " << i << strList.getFirst() << std::endl;
        }
    }
    
    void printNumberList( co::Range<co::int32 const> list )
    {
        for( int i = 0; list; ++i, list.popFirst() )
        {
            std::cout << "PRINT NUMBER ELEMENT : " << i << list.getFirst() << std::endl;
        }
    }
    
    void printHybridList( co::Range<co::int32 const> list, co::Range<std::string const> strList )
    {
        for( int i = 0; list; ++i, list.popFirst() )
        {
            std::cout << "PRINT NUMBER ELEMENT : " << i << list.getFirst() << std::endl;
        }
        
        for( int i = 0; strList; ++i, strList.popFirst() )
        {
            std::cout << "PRINT STRING ELEMENT : " << i << strList.getFirst() << std::endl;
        }
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
        std::cerr << "THIS IS number: " << _number << std::endl;
    }
	
private:
    int _count;
	co::int32 _number;
	
};
CORAL_EXPORT_COMPONENT( Toto, Toto );
};