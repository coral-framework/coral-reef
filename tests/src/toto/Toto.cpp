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
    
    void printStringList( const std::vector<std::string>& strList )
    {
        for( int i = 0; i < strList.size(); ++i )
        {
            std::cout << "PRINT STRING ELEMENT : " << i << " : " << strList[i];
        }
    }
    
    void printNumberList( const std::vector<co::int32>& list )
    {
        for( int i = 0; i < list.size(); ++i )
        {
            std::cout << "PRINT NUMBER ELEMENT : " << i << " : " << list[i];
        }
    }
    
    void printHybridList( const std::vector<co::int32>& list, const std::vector<std::string>& strList )
    {
        for( int i = 0; i < list.size(); ++i )
        {
            std::cout << "PRINT NUMBER ELEMENT Number = " << list[i] << " : " << strList[i];
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
        printMethodCount( _count );
        std::cerr << "THIS IS METHOD3!" << std::endl;
    }
	
private:
    int _count;
	co::int32 _number;
	
};
CORAL_EXPORT_COMPONENT( Toto, Toto );
};