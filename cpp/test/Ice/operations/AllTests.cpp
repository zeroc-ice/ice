// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "test:default -p 12010 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing ice_communicator... " << flush;
    test(base->ice_communicator() == communicator);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
    Ice::LocatorPrx loc = Ice::LocatorPrx::checkedCast(base);
    test(loc == 0);

    //
    // Upcasting
    //
    Test::MyClassPrx cl2 = Test::MyClassPrx::checkedCast(derived);
    Ice::ObjectPrx obj = Ice::ObjectPrx::checkedCast(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    //
    // Now with alternate API
    //
    cl = checkedCast<Test::MyClassPrx>(base);
    test(cl);
    derived = checkedCast<Test::MyDerivedClassPrx>(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
    loc = checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    cout << "ok" << endl;

    cout << "testing checked cast with context... " << flush;
    string cref = "context:default -p 12010 -t 10000";
    Ice::ObjectPrx cbase = communicator->stringToProxy(cref);
    test(cbase);

    Test::TestCheckedCastPrx tccp = Test::TestCheckedCastPrx::checkedCast(cbase);
    Ice::Context c = tccp->getContext();
    test(c.size() == 0);

    c["one"] = "hello";
    c["two"] = "world";
    tccp = Test::TestCheckedCastPrx::checkedCast(cbase, c);
    Ice::Context c2 = tccp->getContext();
    test(c == c2);

    //
    // Now with alternate API
    //
    tccp = checkedCast<Test::TestCheckedCastPrx>(cbase);
    c = tccp->getContext();
    test(c.size() == 0);

    tccp = checkedCast<Test::TestCheckedCastPrx>(cbase, c);
    c2 = tccp->getContext();
    test(c == c2);

    cout << "ok" << endl;

    if(!collocated)
    {
	cout << "testing timeout... " << flush;
	Test::MyClassPrx clTimeout = Test::MyClassPrx::uncheckedCast(cl->ice_timeout(500));
	try
	{
	    clTimeout->opSleep(2000);
	    test(false);
	}
	catch(const Ice::TimeoutException&)
	{
	}
	cout << "ok" << endl;
    }

    cout << "testing twoway operations... " << flush;
    void twoways(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    twoways(communicator, cl);
    twoways(communicator, derived);
    derived->opDerived();
    cout << "ok" << endl;

    if(!collocated)
    {
	cout << "testing twoway operations with AMI... " << flush;
	void twowaysAMI(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
	twowaysAMI(communicator, cl);
	twowaysAMI(communicator, derived);
	cout << "ok" << endl;

	cout << "testing batch oneway operations... " << flush;
	void batchOneways(const Test::MyClassPrx&);
	batchOneways(cl);
	batchOneways(derived);
	cout << "ok" << endl;
    }

    return cl;
}
