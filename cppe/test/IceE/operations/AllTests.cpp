// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/Locator.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(const IceE::CommunicatorPtr& communicator)
{
    tprintf("testing stringToProxy... ");
    string ref = "test:default -p 12345 -t 10000";
    IceE::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    
#ifndef ICE_NO_LOCATOR
    IceE::LocatorPrx loc = IceE::LocatorPrx::checkedCast(base);
    test(loc == 0);
#endif

    //
    // Upcasting
    //
    Test::MyClassPrx cl2 = Test::MyClassPrx::checkedCast(derived);
    IceE::ObjectPrx obj = IceE::ObjectPrx::checkedCast(derived);
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
    
#ifndef ICE_NO_LOCATOR
    loc = checkedCast<IceE::LocatorPrx>(base);
    test(loc == 0);
#endif

    cl2 = checkedCast<Test::MyClassPrx>(derived);
    obj = checkedCast<IceE::ObjectPrx>(derived);
    test(cl2);
    test(obj);
    test(cl2 == obj);
    test(cl2 == derived);

    tprintf("ok\n");

#ifdef UNDEFINED
    tprintf("testing checked cast with context... ");
    string cref = "test:default -p 12346 -t 10000";
    IceE::ObjectPrx cbase = communicator->stringToProxy(cref);
    test(cbase);

    Test::TestCheckedCastPrx tccp = Test::TestCheckedCastPrx::checkedCast(cbase);
    IceE::Context c = tccp->getContext();
    test(c.size() == 0);

    c["one"] = "hello";
    c["two"] = "world";
    tccp = Test::TestCheckedCastPrx::checkedCast(cbase, c);
    IceE::Context c2 = tccp->getContext();
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
#endif
/*
    tprintf("testing twoway operations... ");
    void twoways(const IceE::CommunicatorPtr&, const Test::MyClassPrx&);
    twoways(communicator, cl);
    twoways(communicator, derived);
    derived->opDerived();
    tprintf("ok\n");
    */

    tprintf("testing batch oneway operations... ");
    void batchOneways(const Test::MyClassPrx&);
    batchOneways(cl);
    batchOneways(derived);
    tprintf("ok\n");

    return cl;
}
