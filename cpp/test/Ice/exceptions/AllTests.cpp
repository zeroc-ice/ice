// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

class MyExceptionFactory : public Ice::UserExceptionFactory
{
public:

    virtual void createAndThrow(const string& type)
    {
	if(type == "::A")
	{
	    throw A();
	}
	else if(type == "::B")
	{
	    throw B();
	}
	else if(type == "::C")
	{
	    throw C();
	}
	else if(type == "::D")
	{
	    throw D();
	}
	assert(false); // Should never be reached
    }

    virtual void destroy()
    {
	// Nothing to do
    }
};

ThrowerPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "thrower:default -p 12345 -t 2000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    ThrowerPrx thrower = ThrowerPrx::checkedCast(base);
    test(thrower);
    test(thrower == base);
    cout << "ok" << endl;

    cout << "catching exact types... " << flush;

    try
    {
	thrower->throwAasA(1);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwAorDasAorD(1);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwAorDasAorD(-1);
	test(false);
    }
    catch(const D& ex)
    {
	test(ex.dMem == -1);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
	test(ex.cMem == 3);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    cout << "catching base types... " << flush;

    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    if(!collocated) // If the server is collocated, exception factories are not needed.
    {
	cout << "catching derived types w/o exception factories... " << flush;
	
	try
	{
	    thrower->throwBasA(1, 2);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch (...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwCasA(1, 2, 3);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch (...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwCasB(1, 2, 3);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch (...)
	{
	    test(false);
	}
	
	cout << "ok" << endl;
	
	cout << "catching derived types w/ exception factories... " << flush;
	
	Ice::UserExceptionFactoryPtr factory = new MyExceptionFactory;
	communicator->addUserExceptionFactory(factory, "::A");
	communicator->addUserExceptionFactory(factory, "::B");
	communicator->addUserExceptionFactory(factory, "::C");
	communicator->addUserExceptionFactory(factory, "::D");
    }
    else
    {
	cout << "catching derived types... " << flush;
    }
	
    try
    {
	thrower->throwBasA(1, 2);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasA(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
    	test(ex.bMem == 2);
    	test(ex.cMem == 3);
    }
    catch (...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasB(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
	test(ex.cMem == 3);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    if(thrower->supportsUndeclaredExceptions())
    {
        cout << "catching unknown user exception... " << flush;

        try
        {
            thrower->throwUndeclaredA(1);
            test(false);
        }
        catch(const Ice::UnknownUserException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            cout << ex << endl;
            test(false);
        }
        catch (...)
        {
            test(false);
        }

        try
        {
            thrower->throwUndeclaredB(1, 2);
            test(false);
        }
        catch(const Ice::UnknownUserException&)
        {
        }
        catch (...)
        {
            test(false);
        }

        try
        {
            thrower->throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch(const Ice::UnknownUserException&)
        {
        }
        catch (...)
        {
            test(false);
        }

        cout << "ok" << endl;
    }
    
    cout << "catching object not exist exception... " << flush;

    Ice::Identity id = Ice::stringToIdentity("does not exist");
    try
    {
	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_newIdentity(id));
	thrower2->ice_ping();
	test(false);
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
	test(ex.id == id);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching facet not exist exception... " << flush;

    try
    {
	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
	thrower2->ice_ping();
	test(false);
    }
    catch(const Ice::FacetNotExistException& ex)
    {
	test(ex.facet.front() == "no such facet");
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching operation not exist exception... " << flush;

    try
    {
	WrongOperationPrx thrower2 = WrongOperationPrx::uncheckedCast(thrower);
	thrower2->noSuchOperation();
	test(false);
    }
    catch(const Ice::OperationNotExistException& ex)
    {
	test(ex.operation == "noSuchOperation");
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching unknown local exception... " << flush;

    try
    {
	thrower->throwLocalException();
	test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching unknown non-Ice exception... " << flush;

    try
    {
	thrower->throwNonIceException();
	test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    return thrower;
}
