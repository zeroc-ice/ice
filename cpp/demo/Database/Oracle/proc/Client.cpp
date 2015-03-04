// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    void printDepts(const DeptPrxSeq&) const;
    void printEmps(const EmpPrxSeq&) const;

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
        "dept <number>: set department <number> as the current department\n"
        "emp <number>: set employee <number> as the current employee\n"
        "exit or quit: exit client\n"
        "help: print this list of commands\n"
        "root: go back to the root menu\n";

    _rootCommands =
        "create: create a new department\n"
        "find <name>: find the department(s) with the given name\n"
        "list: list all departments\n";

        
    _deptCommands =
        "create: create a new employee in this department\n"
        "find <name>: find employee(s) named <name> in this department\n" 
        "list: list all employees in this department\n"
        "ping: ping this department\n"
        "remove: remove this department\n"
        "show: describe this department\n"
        "update <dname|loc> <new value>: update this department\n";

    _empCommands =
        "ping: ping this employee\n"
        "remove: remove this employee\n"
        "show: describe this employee\n"
        "update <ename|job|mgr|hiredate|sal|comm|dept> <new-value>: update this employee\n";
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
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->propertyToProxy("HR.DeptFactory");
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
        
        try
        {

            //
            // Common commands
            //
            if(command == "dept")
            {
                int deptno;
                cin >> deptno;
                if(checkCin(command))
                {
                    _currentDept = DeptPrx::uncheckedCast(_factory->findDeptByNo(deptno));
                    if(_currentDept != 0)
                    {
                        _currentMenu = deptMenu;
                    }
                    else
                    {
                        cout << "There is no department with deptno " << deptno << endl;
                    }   
                }       
            }
            else if(command == "emp")
            {
                int empno;
                cin >> empno;
                if(checkCin(command))
                {
                    _currentEmp = EmpPrx::uncheckedCast(_factory->findEmpByNo(empno));
                    if(_currentEmp != 0)
                    {
                        _currentMenu = empMenu;
                    }
                    else
                    {
                        cout << "There is no employee with empno " << empno << endl;
                    }
                }
            }
            else if(command == "exit" || command == "quit")
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
            _factory->createDept(deptno, desc);
            cout << "Created new department number " << deptno << endl; 
        }
    }
    else if(command == "find")
    {
        string name;
        cin >> name;
        if(checkCin(command))
        {
            printDepts(_factory->findByName(name));
        }
    }
    else if(command == "list")
    {
        checkEof(command);
        printDepts(_factory->findAll());
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
        cout << "Please enter: empno ename job mgr(empno) hiredate sal comm ==> ";
        int empno;
        int mgrEmpno;
        EmpDesc desc;
        cin >> empno >> desc.ename >> desc.job >> mgrEmpno >> desc.hiredate >> desc.sal >> desc.comm;
        
        desc.ename = unquote(desc.ename);
        desc.job = unquote(desc.job);

        if(mgrEmpno != 0)
        {
            desc.mgr = _factory->findEmpByNo(mgrEmpno);
            if(desc.mgr == 0)
            {
                cout << "Manager #" << mgrEmpno << " does not exist: clearing manager" << endl; 
            }
        }
        desc.hiredate = unquote(desc.hiredate);
        desc.sal = unquote(desc.sal);
        desc.comm = unquote(desc.comm);

        desc.edept = _currentDept;

        if(checkCin("create parameters"))
        {
            _currentDept->createEmp(empno, desc);
            cout << "Created new employee number " << empno << endl;
        }
    }
    else if(command == "find")
    {
        string name;
        cin >> name;
        if(checkCin(command))
        {
            printEmps(_currentDept->findByName(name));
        }
    }
    else if(command == "list")
    {
        checkEof(command);
        printEmps(_currentDept->findAll());
    }
    else if(command == "ping")
    {
        checkEof(command);
        _currentDept->ice_ping();
        cout << "ice_ping: success!" << endl;
    }
    else if(command == "remove")
    {
        checkEof(command);
        _currentDept->remove();
    }
    else if(command == "show")
    {
        checkEof(command);
        DeptDesc desc = _currentDept->getDesc();
        cout << "deptno: " << desc.deptno << endl;
        cout << "dname: " << quote(desc.dname) << endl;
        cout << "loc: " << quote(desc.loc) << endl;
    }
    else if(command == "update")
    {
        string field;
        string newValue;
        cin >> field >> newValue;
        newValue = unquote(newValue);

        if(checkCin("update " + field))
        {
            _currentDept->updateField(field, newValue);
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
        _currentEmp->ice_ping();
        cout << "ice_ping: success!" << endl;
    }
    else if(command == "remove")
    {
        checkEof(command);
        _currentEmp->remove();
    }
    else if(command == "show")
    {
        checkEof(command);
        EmpDesc desc = _currentEmp->getDesc();
        cout << "empno: " << desc.empno << endl;
        cout << "ename: " << quote(desc.ename) << endl;
        cout << "job: " << quote(desc.job) << endl;
        cout << "mgr: ";
        if(desc.mgr == 0)
        {
            cout << "<null>" << endl;
        }
        else
        {
            cout << desc.mgr->getDesc().empno << endl;
        }
        cout << "hiredate: " << quote(desc.hiredate) << endl;
        cout << "sal: " << quote(desc.sal) << endl;
        cout << "comm: " << quote(desc.comm) << endl;
        cout << "dept: ";
        if(desc.edept == 0)
        {
            cout << "<null>" << endl;
        }
        else
        {
            cout << desc.edept->getDesc().deptno << endl;
        }
    }
    else if(command == "update")
    {
        string field;
        cin >> field;
        if(field == "mgr")
        {
            int mgr;
            cin >> mgr;
            if(checkCin("update mgr"))
            {
                _currentEmp->updateMgr(mgr);
            }
        }
        else if(field == "dept")
        {
            int deptno;
            cin >> deptno;
            if(checkCin("update dept"))
            {
                _currentEmp->updateDept(deptno);
            }
        }
        else
        {
            string newValue;
            cin >> newValue;
            newValue = unquote(newValue);
            if(checkCin("update " + field))
            {
                _currentEmp->updateField(field, newValue);
            }
        }
    }
    else
    {
        invalidCommand(command);
    }
}

void
HRClient::printDepts(const DeptPrxSeq& depts) const
{
    cout << "Deptno\t Dname\t Loc" << endl; 
    if(depts.size() == 0)
    {
        cout << "<None found>" << endl;
    }
    else
    {
        for(DeptPrxSeq::const_iterator p = depts.begin(); p != depts.end(); ++p)
        {
            HR::DeptDesc desc = (*p)->getDesc();
            cout << desc.deptno << "\t " << desc.dname << "\t " << desc.loc << endl;
        }
    }
}

void
HRClient::printEmps(const EmpPrxSeq& emps) const
{
    cout << "Empno\t Ename" << endl; 
    if(emps.size() == 0)
    {
        cout << "<None found>" << endl;
    }
    else
    {
        for(EmpPrxSeq::const_iterator p = emps.begin(); p != emps.end(); ++p)
        {
            HR::EmpDesc desc = (*p)->getDesc();
            cout << desc.empno << "\t " << desc.ename << endl;
        }
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
