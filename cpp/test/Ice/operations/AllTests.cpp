// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

Test::MyClassPrx
allTests(Ice::CommunicatorPtr communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref("test:tcp -p 12345 -t 2000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    Test::MyClassPrx cl = Test::MyClassPrx::checkedCast(base);
    test(cl);
    Test::MyDerivedClassPrx derived = Test::MyDerivedClassPrx::checkedCast(cl);
    test(derived);
    test(cl == base);
    test(derived == base);
    test(cl == derived);
    cout << "ok" << endl;

    cout << "testing twoway operations... " << flush;
    void twoways(Test::MyClassPrx);
    twoways(cl);
    twoways(derived);
    derived->opDerived();
    cout << "ok" << endl;

    cout << "testing exceptions... " << flush;
    void exceptions(Test::MyClassPrx);
    exceptions(cl);
    cout << "ok" << endl;

    return cl;
}
