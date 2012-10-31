/*
 * Calcium - domain Model Framework
 * See copyright notice in LICENSE.md
 */

#include "CompanySpace.h"

#include <co/Log.h>

void CompanySpace::SetUp()
{
	// create an object model
	_modelObj = co::newInstance( "ca.Model" );
	_model = _modelObj->getService<ca::IModel>();
	assert( _model.isValid() );

	_model->setName( "dom" );

	// create an object universe and bind the model
	_universeObj = co::newInstance( "ca.Universe" );
	_universe = _universeObj->getService<ca::IUniverse>();
	assert( _universe.isValid() );

	_universeObj->setService( "model", _model.get() );

	// create an object space and bind it to the universe
	_spaceObj = co::newInstance( "ca.Space" );
	_space = _spaceObj->getService<ca::ISpace>();
	assert( _space.isValid() );

	_spaceObj->setService( "universe", _universe.get() );

	_space->initialize( createCompanyGraph() );
	_space->notifyChanges();

}

void CompanySpace::TearDown()
{
	//_space->removeSpaceObserver( this );

	_modelObj = NULL;
	_spaceObj = NULL;
	_universeObj = NULL;

	_model = NULL;
	_space = NULL;
	_universe = NULL;

	_manager1 = NULL;
	_manager2 = NULL;

	_developer1 = NULL;
	_developer2 = NULL;
	_developer3 = NULL;
	_developer4 = NULL;

	_software = NULL;
	_dataMaintain = NULL;
	
	_company = NULL;
}

co::IObject* CompanySpace::createCompanyGraph()
{
	_developer1 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_developer1->setName( "John Cplusplus Experienced" );
	_developer1->setSalary( 5000 );
	_developer1->setRole( "Developer" );
	
	_developer2 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_developer2->setName( "Joseph Java Newbie" );
	_developer2->setSalary( 1000 );
	_developer2->setRole( "Developer" );

	_developer3 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_developer3->setName( "Michael CSharp Senior" );
	_developer3->setSalary( 4000 );
	_developer3->setRole( "Developer" );

	_developer4 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_developer4->setName( "Jacob Lua Junior" );
	_developer4->setSalary( 3000 );
	_developer4->setRole( "Developer" );

	_manager1 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_manager1->setName( "Richard Scrum Master" );
	_manager1->setSalary( 10000 );
	_manager1->setRole( "Manager" );
	
	_software = co::newInstance( "dom.Product" )->getService<dom::IProduct>();
	_software->setName( "Software2.0" );
	_software->setValue( 1000000 );

	_manager1->setLeading( _software.get() );

	_manager2 = co::newInstance( "dom.Employee" )->getService<dom::IEmployee>();
	_manager2->setName( "Wiliam Kanban Expert" );
	_manager2->setSalary( 9000 );

	std::vector<dom::IProject*> working;
	working.push_back( _software.get() );
	_developer2->setWorking( working );
	_developer3->setWorking( working );

	_dataMaintain = co::newInstance( "dom.Service" )->getService<dom::IService>();
	_dataMaintain->setName( "Software1.0 Maintenance" );
	_dataMaintain->setMonthlyIncome( 50000 );

	std::vector<dom::IEmployeeRef> maintainers;
	maintainers.push_back( _developer1.get() );
	maintainers.push_back( _developer4.get() );

	working.clear();
	working.push_back( _dataMaintain.get() );

	_developer1->setWorking( working );
	_developer4->setWorking( working );

	_company = co::newInstance( "dom.Company" )->getService<dom::ICompany>();

	std::vector<dom::IEmployee*> employees;
	employees.push_back( _developer2.get() );
	employees.push_back( _developer3.get() );
	employees.push_back( _manager1.get() );
	employees.push_back( _developer1.get() );
	employees.push_back( _developer4.get() );
	
	_company->setEmployees( employees );

	return _company->getProvider();
}

