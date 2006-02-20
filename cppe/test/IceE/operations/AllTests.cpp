// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault(
	"Operations.Proxy", "test:default -p 12010 -t 10000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing ice_communicator... ");
    test(base->ice_communicator().get() == communicator.get());
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
#ifdef ICEE_HAS_LOCATOR
    Ice::LocatorPrx loc = Ice::LocatorPrx::checkedCast(base);
    test(loc == 0);
#endif

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
    
#ifdef ICEE_HAS_LOCATOR
    loc = checkedCast<Ice::LocatorPrx>(base);
    test(loc == 0);
#endif

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<Ice::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    tprintf("ok\n");

    tprintf("testing checked cast with context...");
    ref = communicator->getProperties()->getPropertyWithDefault(
	"Operations.ContextProxy", "context:default -p 12010 -t 10000");
    Ice::ObjectPrx cbase = communicator->stringToProxy(ref);
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

    tprintf("ok\n");
//XXXX:

    tprintf("testing timeout...");
    Test::MyClassPrx clTimeout = Test::MyClassPrx::uncheckedCast(cl->ice_timeout(500));
    try
    {
	clTimeout->opSleep(1000);
	assert(false);
    }
    catch(const Ice::TimeoutException&)
    {
    }
    tprintf("ok\n");

    tprintf("testing twoway operations... ");
    void twoways(const Ice::CommunicatorPtr&, const Test::MyClassPrx&);
    twoways(communicator, cl);
    twoways(communicator, derived);
    derived->opDerived();
    tprintf("ok\n");

    tprintf("testing batch oneway operations... ");
    void batchOneways(const Test::MyClassPrx&);
    batchOneways(cl);
    batchOneways(derived);
    tprintf("ok\n");

    return cl;
}
