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

TestIntfPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    string ref = communicator->getProperties()->getPropertyWithDefault("Slicing.Proxy", "Test:default -p 12010");
    Ice::ObjectPrx obj = communicator->stringToProxy(ref);
    TestIntfPrx test = TestIntfPrx::checkedCast(obj);

    tprintf("base... "); fflush(stdout);
    {
	try
	{
	    test->baseAsBase();
	    test(false);
	}
	catch(const Base& b)
	{
	    test(b.b == "Base.b");
	    test(b.ice_name() =="Test::Base");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of unknown derived... "); fflush(stdout);
    {
	try
	{
	    test->unknownDerivedAsBase();
	    test(false);
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownDerived.b");
	    test(b.ice_name() =="Test::Base");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as base... "); fflush(stdout);
    {
	try
	{
	    test->knownDerivedAsBase();
	    test(false);
	}
	catch(const KnownDerived& k)
	{
	    test(k.b == "KnownDerived.b");
	    test(k.kd == "KnownDerived.kd");
	    test(k.ice_name() =="Test::KnownDerived");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("non-slicing of known derived as derived... "); fflush(stdout);
    {
	try
	{
	    test->knownDerivedAsKnownDerived();
	    test(false);
	}
	catch(const KnownDerived& k)
	{
	    test(k.b == "KnownDerived.b");
	    test(k.kd == "KnownDerived.kd");
	    test(k.ice_name() =="Test::KnownDerived");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of unknown intermediate as base... "); fflush(stdout);
    {
	try
	{
	    test->unknownIntermediateAsBase();
	    test(false);
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownIntermediate.b");
	    test(b.ice_name() =="Test::Base");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of known intermediate as base... "); fflush(stdout);
    {
	try
	{
	    test->knownIntermediateAsBase();
	    test(false);
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "KnownIntermediate.b");
	    test(ki.ki == "KnownIntermediate.ki");
	    test(ki.ice_name() =="Test::KnownIntermediate");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of known most derived as base... "); fflush(stdout);
    {
	try
	{
	    test->knownMostDerivedAsBase();
	    test(false);
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() =="Test::KnownMostDerived");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("non-slicing of known intermediate as intermediate... "); fflush(stdout);
    {
	try
	{
	    test->knownIntermediateAsKnownIntermediate();
	    test(false);
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "KnownIntermediate.b");
	    test(ki.ki == "KnownIntermediate.ki");
	    test(ki.ice_name() =="Test::KnownIntermediate");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived exception as intermediate... "); fflush(stdout);
    {
	try
	{
	    test->knownMostDerivedAsKnownIntermediate();
	    test(false);
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() =="Test::KnownMostDerived");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("non-slicing of known most derived as most derived... "); fflush(stdout);
    {
	try
	{
	    test->knownMostDerivedAsKnownMostDerived();
	    test(false);
	}
	catch(const KnownMostDerived& kmd)
	{
	    test(kmd.b == "KnownMostDerived.b");
	    test(kmd.ki == "KnownMostDerived.ki");
	    test(kmd.kmd == "KnownMostDerived.kmd");
	    test(kmd.ice_name() =="Test::KnownMostDerived");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as base... "); fflush(stdout);
    {
	try
	{
	    test->unknownMostDerived1AsBase();
	    test(false);
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "UnknownMostDerived1.b");
	    test(ki.ki == "UnknownMostDerived1.ki");
	    test(ki.ice_name() =="Test::KnownIntermediate");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, known intermediate as intermediate... "); fflush(stdout);
    {
	try
	{
	    test->unknownMostDerived1AsKnownIntermediate();
	    test(false);
	}
	catch(const KnownIntermediate& ki)
	{
	    test(ki.b == "UnknownMostDerived1.b");
	    test(ki.ki == "UnknownMostDerived1.ki");
	    test(ki.ice_name() =="Test::KnownIntermediate");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    tprintf("slicing of unknown most derived, unknown intermediate as base... "); fflush(stdout);
    {
	try
	{
	    test->unknownMostDerived2AsBase();
	    test(false);
	}
	catch(const Base& b)
	{
	    test(b.b == "UnknownMostDerived2.b");
	    test(b.ice_name() =="Test::Base");
	}
	catch(...)
	{
	    test(false);
	}
    }
    tprintf("ok\n");

    return test;
}
