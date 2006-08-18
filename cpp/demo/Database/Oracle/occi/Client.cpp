// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HR.h>
#include <iostream>
#include <limits>

using namespace std;
using namespace HR;

class HRClient : public Ice::Application
{
public:
    HRClient();

    virtual int run(int, char*[]);

    enum Menu {rootMenu, empMenu, deptMenu}; 

private:

    void help() const;
    bool checkCin(const string& command) const;
    void checkEof(const string& command) const;
    void invalidCommand(const string& command) const;

    void doRootMenu(const string&) const;
    void doDeptMenu(const string&) const;
    void doEmpMenu(const string&) const;

    static string quote(const string&);
    static string unquote(const string&);
    
    Menu _currentMenu;
    DeptFactoryPrx _factory;
    DeptPrx _currentDept;
    EmpPrx _currentEmp;

    string _commonCommands;
    string _rootCommands;
    string _deptCommands;
    string _empCommands;
};

int
main(int argc, char* argv[])
{
    HRClient app;
    return app.main(argc, argv, "config.client");
}

HRClient::HRClient() :
    _currentMenu(rootMenu)
{
    _commonCommands =
	"dept <identity>: set department <identity> as the current department\n"
	"emp <identity>: set employee <identity> as the current employee\n"
	"exit: exit client\n"
	"help: print this list of commands\n"
	"root: go back to the root menu\n";

    _rootCommands =
	"create: create a new department\n"
	"find <name>: find the deparment(s) with the given name\n"
	"list: list all departments\n";

	
    _deptCommands =
	"create: create a new employee in this department\n"
	"find <name>: find employee(s) named <name> in this department\n" 
	"list: list all employees in this department\n"
	"ping: ping this department\n"
	"remove: remove this department\n"
	"show: describe this department\n"
	"update: update this department\n";

    _empCommands =
	"ping: ping this employee\n"
	"remove: remove this employee\n"
	"show: describe this employee\n"
	"update: update this employee\n";
}

void
HRClient::checkEof(const string& command) const
{
    if(!cin.eof())
    {
	string extra;
	getline(cin, extra);
	if(extra.size() > 0)
	{
	    cout << "Warning: ignoring extra args '" << extra 
		 << "' for '" << command << "'" << endl;
	}
    }
}

bool
HRClient::checkCin(const string& command) const
{
    if(!cin)
    {
	cout << "Error: failed to read arguments for '" << command << "'" << endl;
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return false;
    }
    checkEof(command);
    return true;
}

void
HRClient::invalidCommand(const string& command) const
{
    cout << "Invalid command '" << command << "'. "
	"Type 'help' for help." << endl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void
HRClient::help() const
{
    cout << "Commands are:\n";
    cout << "--- Specific to this menu ---\n";

    switch(_currentMenu)
    {
	case rootMenu:
	{
	    cout << _rootCommands;
	    break;
	}
	case deptMenu:
	{
	    cout << _deptCommands;
	    break;
	}
	case empMenu:
	{
	    cout << _empCommands;
	    break;
	}
    }
    cout << "--- Common to all menus ---\n";
    cout << _commonCommands << endl;
}

int
HRClient::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const string proxyProperty = "HR.DeptFactory";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->stringToProxy(proxy);
    _factory = DeptFactoryPrx::checkedCast(base);
    if(_factory == 0)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }
   
    Ice::EndpointSeq endpoints = _factory->ice_getEndpoints(); 


    //
    // cin loop
    //

    string command;
    
    do
    {
	cout << "==> ";
	cin >> command;

	if(!cin)
	{
	    break;
	}

	//
	// Common commands
	//
	if(command == "dept")
	{
	    string identity;
	    cin >>  identity;
	    if(checkCin(command))
	    {
		_currentMenu = deptMenu;
		_currentDept = DeptPrx::uncheckedCast(
		    communicator()->stringToProxy(identity)->ice_endpoints(endpoints));
	    }	
	}
	else if(command == "emp")
	{
	    string identity;
	    cin >>  identity;
	    if(checkCin(command))
	    {
		_currentMenu = empMenu;
		_currentEmp = EmpPrx::uncheckedCast(
		    communicator()->stringToProxy(identity)->ice_endpoints(endpoints));
	    }
	}
	else if(command == "exit")
	{
	    checkEof(command);
	    break;
	}
	else if(command == "help")
	{
	    checkEof(command);
	    help();
	}
	else if(command == "root")
	{
	    checkEof(command);
	    _currentMenu = rootMenu;
	}
	else if(_currentMenu == rootMenu)
	{
	    doRootMenu(command);
	}
	else if(_currentMenu == deptMenu)
	{
	    doDeptMenu(command);
	}
	else if(_currentMenu == empMenu)
	{
	    doEmpMenu(command);
	}
	else
	{
	    assert(0);
	}
    }
    while(cin.good());

    return EXIT_SUCCESS;
}

void
HRClient::doRootMenu(const string& command) const
{
    if(command == "create")
    {
	checkEof(command);
	cout << "Please enter: deptno dname loc ==> ";
	int deptno;
	DeptDesc desc;
	cin >> deptno >> desc.dname >> desc.loc;

	desc.dname = unquote(desc.dname);
	desc.loc = unquote(desc.loc);

	if(checkCin("create parameters"))
	{
	    try
	    {
		_factory->createDept(deptno, desc);
		cout << "Created new department number " << deptno << endl;
	    }
	    catch(const SqlException& e)
	    {
		cout << "Caught a SqlException: " << e.reason << endl;
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else if(command == "find")
    {
	string name;
	cin >> name;
	if(checkCin(command))
	{
	    DeptPrxSeq depts;
	    try
	    {
		depts = _factory->findByName(name);
		if(depts.size() == 0)
		{
		    cout << "<None found>" << endl;
		}
		else
		{
		    for(DeptPrxSeq::iterator p = depts.begin(); p != depts.end(); ++p)
		    {
			cout << communicator()->identityToString((*p)->ice_getIdentity()) << endl;
		    }
		}
		
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else if(command == "list")
    {
	checkEof(command);
	DeptPrxSeq depts;
	try
	{
	    depts = _factory->findAll();
	    if(depts.size() == 0)
	    {
		cout << "<None found>" << endl;
	    }
	    else
	    {
		for(DeptPrxSeq::iterator p = depts.begin(); p != depts.end(); ++p)
		{
		    cout << communicator()->identityToString((*p)->ice_getIdentity()) << endl;
		}
	    }
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else
    {
	invalidCommand(command);
    }
}

void
HRClient::doDeptMenu(const string& command) const
{
    if(command == "create")
    {
	checkEof(command);
	cout << "Please enter: empno ename job mgr hiredate sal comm ==> ";
	int empno;
	string mgrId;
	EmpDesc desc;
	cin >> empno >> desc.ename >> desc.job >> mgrId >> desc.hiredate >> desc.sal >> desc.comm;
	
	desc.ename = unquote(desc.ename);
	desc.job = unquote(desc.job);

	if(mgrId != "''")
	{
	    desc.mgr = EmpPrx::uncheckedCast(communicator()
					     ->stringToProxy(mgrId)->ice_endpoints(
						 _factory->ice_getEndpoints()));
	}
	desc.hiredate = unquote(desc.hiredate);
	desc.sal = unquote(desc.sal);
	desc.comm = unquote(desc.comm);

	desc.edept = _currentDept;

	if(checkCin("create parameters"))
	{
	    try
	    {
		_currentDept->createEmp(empno, desc);
		cout << "Created new employee number " << empno << endl;
	    }
	    catch(const SqlException& e)
	    {
		cout << "Caught a SqlException: " << e.reason << endl;
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else if(command == "find")
    {
	string name;
	cin >> name;
	if(checkCin(command))
	{
	    EmpPrxSeq emps;
	    try
	    {
		emps = _currentDept->findByName(name);
		if(emps.size() == 0)
		{
		    cout << "<None found>" << endl;
		}
		else
		{
		    for(EmpPrxSeq::iterator p = emps.begin(); p != emps.end(); ++p)
		    {
			cout << communicator()->identityToString((*p)->ice_getIdentity()) << endl;
		    }
		}
		
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else if(command == "list")
    {
	checkEof(command);
	EmpPrxSeq emps;
	try
	{
	    emps = _currentDept->findAll();
	    if(emps.size() == 0)
	    {
		cout << "<None found>" << endl;
	    }
	    else
	    {
		for(EmpPrxSeq::iterator p = emps.begin(); p != emps.end(); ++p)
		{
		    cout << communicator()->identityToString((*p)->ice_getIdentity()) << endl;
		}
	    }
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else if(command == "ping")
    {
	checkEof(command);
	try
	{
	    _currentDept->ice_ping();
	    cout << "ice_ping: success!" << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}

    }
    else if(command == "remove")
    {
	checkEof(command);
	try
	{
	    _currentDept->remove();
	}
	catch(const SqlException& e)
	{
	    cout << "Caught a SqlException: " << e.reason << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else if(command == "show")
    {
	checkEof(command);
	try
	{
	    DeptDesc desc = _currentDept->getDesc();
	    cout << "identity: " << communicator()->identityToString(
		_currentDept->ice_getIdentity()) << endl;
	    cout << "dname: " << quote(desc.dname) << endl;
	    cout << "loc: " << quote(desc.loc) << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else if(command == "update")
    {
	checkEof(command);
	cout << "Please enter: dname loc ==> ";
	DeptDesc desc;
	cin >> desc.dname >> desc.loc;
	desc.dname = unquote(desc.dname);
	desc.loc = unquote(desc.loc);

	if(checkCin("update parameters"))
	{
	    try
	    {
		_currentDept->updateDesc(desc);
		cout << "Department updated" << endl;
	    }
	    catch(const SqlException& e)
	    {
		cout << "Caught a SqlException: " << e.reason << endl;
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else
    {
	invalidCommand(command);
    }
}

void
HRClient::doEmpMenu(const string& command) const
{
    if(command == "ping")
    {
	checkEof(command);
	try
	{
	    _currentEmp->ice_ping();
	    cout << "ice_ping: success!" << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}

    }
    else if(command == "remove")
    {
	checkEof(command);
	try
	{
	    _currentEmp->remove();
	}
	catch(const SqlException& e)
	{
	    cout << "Caught a SqlException: " << e.reason << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else if(command == "show")
    {
	checkEof(command);
	try
	{
	    EmpDesc desc = _currentEmp->getDesc();
	    cout << "identity: " << communicator()->identityToString(
		_currentEmp->ice_getIdentity()) << endl;
	    cout << "ename: " << quote(desc.ename) << endl;
	    cout << "job: " << quote(desc.job) << endl;
	    cout << "mgr: " <<  (desc.mgr == 0 ? "''" :
		communicator()->identityToString(
		    desc.mgr->ice_getIdentity())) << endl;
	    cout << "hiredate: " << quote(desc.hiredate) << endl;
	    cout << "sal: " << quote(desc.sal) << endl;
	    cout << "comm: " << quote(desc.comm) << endl;
	    cout << "dept: " << (desc.edept == 0 ? "''" :
		communicator()->identityToString(
		    desc.edept->ice_getIdentity())) << endl;
	}
	catch(const IceUtil::Exception& e)
	{
	    cout << "Caught an Ice exception: " << e << endl;
	}
	catch(const std::exception& e)
	{
	    cout << "Caught a std::exception: " << e.what() << endl;
	}
	catch(...)
	{
	    cout << "Caught an unknown exception" << endl;
	}
    }
    else if(command == "update")
    {
	checkEof(command);
	cout << "Please enter: ename job mgr hiredate sal comm dept ==> ";
	EmpDesc desc;
	string mgrId;
	string deptId;
	cin >> desc.ename >> desc.job 
	    >> mgrId >> desc.hiredate >> desc.sal >> desc.comm >> deptId;
	
	desc.ename = unquote(desc.ename);
	desc.job = unquote(desc.job);

	if(mgrId != "''")
	{
	    desc.mgr = EmpPrx::uncheckedCast(communicator()
					     ->stringToProxy(mgrId)->ice_endpoints(
						 _factory->ice_getEndpoints()));
	}

	cout << "mgrId is " << mgrId << endl;

	desc.hiredate = unquote(desc.hiredate);
	desc.sal = unquote(desc.sal);
	desc.comm = unquote(desc.comm);

	if(deptId != "''")
	{
	    desc.edept = DeptPrx::uncheckedCast(communicator()
						->stringToProxy(deptId)->ice_endpoints(
						    _factory->ice_getEndpoints()));
	}

	if(checkCin("update parameters"))
	{
	    try
	    {
		_currentEmp->updateDesc(desc);
		cout << "Employee updated" << endl;
	    }
	    catch(const SqlException& e)
	    {
		cout << "Caught a SqlException: " << e.reason << endl;
	    }
	    catch(const IceUtil::Exception& e)
	    {
		cout << "Caught an Ice exception: " << e << endl;
	    }
	    catch(const std::exception& e)
	    {
		cout << "Caught a std::exception: " << e.what() << endl;
	    }
	    catch(...)
	    {
		cout << "Caught an unknown exception" << endl;
	    }
	}
    }
    else
    {
	invalidCommand(command);
    }
}

/*static*/ string
HRClient::quote(const string& str)
{
    if(str == "")
    {
	return "''";
    }
    else
    {
	return str;
    }

}

/*static*/ string
HRClient::unquote(const string& str)
{
    if(str == "''")
    {
	return "";
    }
    else
    {
	return str;
    }

}
