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

InitialPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref("initial:tcp -p 12345 -t 2000");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrx initial = InitialPrx::checkedCast(base);
    test(initial);
    test(initial == base);
    cout << "ok" << endl;

    cout << "getting first B... " << flush;
    BPtr b1 = initial->getB1();
    test(b1);
    cout << "ok" << endl;

    cout << "getting second B... " << flush;
    BPtr b2 = initial->getB2();
    test(b2);
    cout << "ok" << endl;

    cout << "getting C... " << flush;
    CPtr c = initial->getC();
    test(c);
    cout << "ok" << endl;

    cout << "getting D... " << flush;
    DPtr d = initial->getD();
    test(d);
    cout << "ok" << endl;

    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    cout << "ok" << endl;

    return initial;
}
