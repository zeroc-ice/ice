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

ThrowerPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref("thrower:tcp -p 12345 -t 2000");
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
	test(ex.a == 1);
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
	test(ex.c == 3);
    }
    catch(...)
    {
	assert(false);
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
	test(ex.a == 1);
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
    }
    catch(...)
    {
	assert(false);
    }
    cout << "ok" << endl;

    return thrower;
}
