// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

TestPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectPrx obj = communicator->stringToProxy("Test:default -p 12345");
    TestPrx test = TestPrx::checkedCast(obj);

    cout << "testing throwing a base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->baseAsBase();
	}
	catch(const Base& b)
	{
	    test(b.b == "Base.b");
	    test(b.ice_name() == "Base");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of unknown derived exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->unknownDerivedAsBase();
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownDerived.b");
	    test(b.ice_name() == "Base");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing non-slicing of known derived exception thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownDerivedAsBase();
	}
	catch(const KnownDerived& k)
	{
	    test(k.b == "KnownDerived.b");
	    test(k.kd == "KnownDerived.kd");
	    test(k.ice_name() == "KnownDerived");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing non-slicing of known derived exception thrown as derived exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownDerivedAsKnownDerived();
	}
	catch(const KnownDerived& k)
	{
	    test(k.b == "KnownDerived.b");
	    test(k.kd == "KnownDerived.kd");
	    test(k.ice_name() == "KnownDerived");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of unknown intermediate exception thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->unknownIntermediateAsBase();
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownIntermediate.b");
	    test(b.ice_name() == "Base");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of known intermediate exception thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownIntermediateAsBase();
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "KnownIntermediate.b");
	    test(ki.ki == "KnownIntermediate.ki");
	    test(ki.ice_name() == "KnownIntermediate");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of known most derived exception thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownMostDerivedAsBase();
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() == "KnownMostDerived");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing non-slicing of known intermediate exception thrown as intermediate exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownIntermediateAsknownIntermediate();
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "KnownIntermediate.b");
	    test(ki.ki == "KnownIntermediate.ki");
	    test(ki.ice_name() == "KnownIntermediate");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing non-slicing of known most derived thrown as intermediate exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownMostDerivedAsKnownIntermediate();
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() == "KnownMostDerived");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing non-slicing of known most derived thrown as most derived exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->knownMostDerivedAsKnownMostDerived();
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() == "KnownMostDerived");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of unknown most derived with known intermediate thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->unknownMostDerived1AsBase();
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "UnknownMostDerived1.b");
	    test(ki.ki == "UnknownMostDerived1.ki");
	    test(ki.ice_name() == "KnownIntermediate");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of unknown most derived with known intermediate thrown as intermediate exception... "
	 << flush;
    {
	bool gotException = false;
	try
	{
	    test->unknownMostDerived1AsKnownIntermediate();
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "UnknownMostDerived1.b");
	    test(ki.ki == "UnknownMostDerived1.ki");
	    test(ki.ice_name() == "KnownIntermediate");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing slicing of unknown most derived with unknown intermediate thrown as base exception... " << flush;
    {
	bool gotException = false;
	try
	{
	    test->unknownMostDerived2AsBase();
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownMostDerived2.b");
	    test(b.ice_name() == "Base");
	    gotException = true;
	}
	catch(...)
	{
	    test(0);
	}
	test(gotException);
    }
    cout << "ok" << endl;

    return test;
}
