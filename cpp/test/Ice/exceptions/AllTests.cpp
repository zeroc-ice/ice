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
allTests(Ice::CommunicatorPtr communicator)
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

    cout << "catching exact types by value... " << flush;
    try
    {
	thrower->throwAasA(1);
	test(false);
    }
    catch(const APtrE& ex)
    {
	test(ex->a == 1);
	const APtr& p = ex;
	test(p->a == 1);
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
    catch(const BPtrE& ex)
    {
	test(ex->a == 1);
	test(ex->b == 2);
	const BPtr& p = ex;
	test(p->a == 1);
	test(p->b == 2);
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
    catch(const CPtrE& ex)
    {
	test(ex->a == 1);
	test(ex->b == 2);
	test(ex->c == 3);
	const CPtr& p = ex;
	test(p->a == 1);
	test(p->b == 2);
	test(p->c == 3);
    }
    catch(...)
    {
	assert(false);
    }
    cout << "ok" << endl;

    cout << "catching base types by value... " << flush;
    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const APtrE& ex)
    {
	test(ex->a == 1);
	const APtr& p = ex;
	test(p->a == 1);
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
    catch(const BPtrE& ex)
    {
	test(ex->a == 1);
	test(ex->b == 2);
	const BPtr& p = ex;
	test(p->a == 1);
	test(p->b == 2);
    }
    catch(...)
    {
	assert(false);
    }
    cout << "ok" << endl;

    cout << "catching exact types by proxy... " << flush;
    try
    {
	thrower->throwAasAproxy();
	test(false);
    }
    catch(const APrxE&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwBasBproxy();
	test(false);
    }
    catch(const BPrxE&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwCasCproxy();
	test(false);
    }
    catch(const CPrxE&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    cout << "ok" << endl;

    cout << "catching base types by proxy... " << flush;
    try
    {
	thrower->throwBasBproxy();
	test(false);
    }
    catch(const APrxE&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	thrower->throwCasCproxy();
	test(false);
    }
    catch(const BPrxE&)
    {
    }
    catch(...)
    {
	assert(false);
    }
    cout << "ok" << endl;

    return thrower;
}
