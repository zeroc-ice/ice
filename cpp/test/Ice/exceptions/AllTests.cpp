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
    catch (const A& ex)
    {
	test(ex.a == 1);
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
    catch (const A& ex)
    {
	test(ex.a == 1);
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
    catch (const D& ex)
    {
	test(ex.d == -1);
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
    catch (const B& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
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
    catch (const C& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
	test(ex.c == 3);
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
    catch (const A& ex)
    {
	test(ex.a == 1);
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
    catch (const B& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    cout << "catching derived types w/o exception factories... " << flush;

    try
    {
	thrower->throwBasA(1, 2);
	test(false);
    }
    catch (const A& ex)
    {
	test(ex.a == 1);
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
    catch (const A& ex)
    {
	test(ex.a == 1);
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
    catch (const B& ex)
    {
	test(ex.a == 1);
	test(ex.b == 2);
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    cout << "catching unknown user exception... " << flush;

    try
    {
	thrower->throwUndeclaredA(1);
	test(false);
    }
    catch (const Ice::UnknownUserException&)
    {
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
    catch (const Ice::UnknownUserException&)
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
    catch (const Ice::UnknownUserException&)
    {
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
    catch (const Ice::UnknownLocalException&)
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
    catch (const Ice::UnknownException&)
    {
    }
    catch (...)
    {
	test(false);
    }

    cout << "ok" << endl;

    return thrower;
}
