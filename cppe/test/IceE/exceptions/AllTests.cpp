// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class EmptyI : virtual public Empty
{
};

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
	_called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    bool check()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	while(!_called)
	{
	    if(!timedWait(IceUtil::Time::seconds(5)))
	    {
		return false;
	    }
	}
	_called = false;
	return true;
    }

protected:

    void called()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	assert(!_called);
	_called = true;
	notify();
    }

private:

    bool _called;
};

ThrowerPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing toString()/what()... ");
    {
        A a;
        string aMsg = "Test::A";

        Ice::UnknownLocalException ule("thisFile", 99);
        string uleMsg = "thisFile:99: Ice::UnknownLocalException:\nunknown local exception";

        //
        // Test toString().
        //
        test(a.toString() == aMsg);
        test(ule.toString() == uleMsg);

        //
        // Test what(). (Called twice because of lazy initialization in what().)
        //
        test(aMsg == a.what());
        test(aMsg == a.what());

        test(uleMsg == ule.what());
        test(uleMsg == ule.what());
    }
    tprintf("ok\n");

    tprintf("testing object adapter registration exceptions... ");
    {
	Ice::ObjectAdapterPtr first = communicator->createObjectAdapter("TestAdapter0");
	try
	{
	    Ice::ObjectAdapterPtr second = communicator->createObjectAdapter("TestAdapter0");
	    test(false);
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    // Expected
	}

	communicator->getProperties()->setProperty("TestAdapter0.Endpoints", "");
	try
	{
	    Ice::ObjectAdapterPtr second = 
		communicator->createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011 -t 10000");
	    test(false);
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    // Expected.
	}
	//
	// Properties must remain unaffected if an exception occurs.
	//
	test(communicator->getProperties()->getProperty("TestAdapter0.Endpoints") == "");
	first->deactivate();
    }
    tprintf("ok\n");

    tprintf("testing servant registration exceptions...");
    {
	Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter1");
	Ice::ObjectPtr obj = new EmptyI;
	adapter->add(obj, communicator->stringToIdentity("x"));
	try
	{
	    adapter->add(obj, communicator->stringToIdentity("x"));
	    test(false);
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	}

	adapter->remove(communicator->stringToIdentity("x"));
	try
	{
	    adapter->remove(communicator->stringToIdentity("x"));
	    test(false);
	}
	catch(const Ice::NotRegisteredException&)
	{
	}

	adapter->deactivate();
    }
    tprintf("ok\n");

    tprintf("testing stringToProxy...");
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Exception.Proxy", "thrower:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast...");
    ThrowerPrx thrower = ThrowerPrx::checkedCast(base);
    test(thrower);
    test(thrower == base);
    tprintf("ok\n");

    tprintf("catching exact types...");

    try
    {
	thrower->throwAasA(1);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(const Ice::Exception& ex)
    {
	tprintf("%s\n", ex.toString().c_str());
	test(false);
    }
    catch(...)
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
    catch(...)
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
    catch(...)
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
    catch(...)
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
    catch(...)
    {
	test(false);
    }

#if (!defined(_MSC_VER) || _MSC_VER >= 1300)
//
// With VC6 SP5, there is no way to call ::A::__write from ::Mod::A
//
    try
    {
	thrower->throwModA(1, 2);
	test(false);
    }
    catch(const Mod::A& ex)
    {
	test(ex.aMem == 1);
	test(ex.a2Mem == 2);
    }
    catch(const Ice::OperationNotExistException&)
    {
	//
        // This operation is not supported in Java.
        //
    }
    catch(...)
    {
	test(false);
    }
#endif

    tprintf("ok\n");

    tprintf("catching base types...");

    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(...)
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
    catch(...)
    {
	test(false);
    }

#if (!defined(_MSC_VER) || _MSC_VER >= 1300)
//
// With VC6 SP5, there is no way to call ::A::__write from ::Mod::A
//
    try
    {
	thrower->throwModA(1, 2);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(const Ice::OperationNotExistException&)
    {
	//
        // This operation is not supported in Java.
        //
    }
    catch(...)
    {
	test(false);
    }
#endif

    tprintf("ok\n");

    tprintf("catching derived types...");
	
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
    catch(...)
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
    catch(...)
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
    catch(...)
    {
	test(false);
    }

    tprintf("ok\n");

    if(thrower->supportsUndeclaredExceptions())
    {
	tprintf("catching unknown user exception...");
	
	try
	{
	    thrower->throwUndeclaredA(1);
	    test(false);
	}
	catch(const Ice::UnknownUserException&)
	{
	}
	catch(...)
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
	catch(...)
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
	catch(...)
	{
	    test(false);
	}
	
	tprintf("ok\n");
    }

    tprintf("catching object not exist exception...");

    Ice::Identity id = communicator->stringToIdentity("does not exist");
    try
    {
	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_identity(id));
	thrower2->throwAasA(1);
//	thrower2->ice_ping();
	test(false);
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
	test(ex.id == id);
    }
    catch(...)
    {
	test(false);
    }

    tprintf("ok\n");
    
    tprintf("catching facet not exist exception...");

    ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
    try
    {
	thrower2->ice_ping();
	test(false);
    }
    catch(const Ice::FacetNotExistException& ex)
    {
	test(ex.facet == "no such facet");
    }
    tprintf("ok\n");
    
    tprintf("catching operation not exist exception...");

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
    catch(...)
    {
	test(false);
    }

    tprintf("ok\n");
    
    tprintf("catching unknown local exception...");

    try
    {
	thrower->throwLocalException();
	test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch(...)
    {
	test(false);
    }

    tprintf("ok\n");
    
    tprintf("catching unknown non-Ice exception...");
    
    try
    {
	thrower->throwNonIceException();
	test(false);
    }
    catch(const Ice::UnknownException&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    
    tprintf("ok\n");

    return thrower;
}
