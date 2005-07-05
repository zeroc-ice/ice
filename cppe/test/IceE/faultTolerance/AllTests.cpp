// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class CallbackBase : public IceE::Monitor<IceE::Mutex>
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
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	while(!_called)
	{
	    if(!timedWait(IceE::Time::seconds(30)))
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
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	assert(!_called);
	_called = true;
	notify();
    }

private:

    bool _called;
};

void
allTests(const IceE::CommunicatorPtr& communicator, const vector<int>& ports)
{
    tprintf("testing stringToProxy...");
    string ref("test");

    char buf[32];
    for(vector<int>::const_iterator p = ports.begin(); p != ports.end(); ++p)
    {
        sprintf(buf, ":default -t 60000 -p %d", *p);
	ref += buf;
    }
    IceE::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast...");
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    tprintf("ok\n");

    int oldPid = 0;
    for(unsigned int i = 1, j = 0; i <= ports.size(); ++i, ++j)
    {
	if(j > 3)
	{
	    j = 0;
	}

	tprintf("testing server #%d...", i);
	int pid = obj->pid();
	test(pid != oldPid);
	tprintf("ok\n");
	oldPid = pid;

	if(j == 0)
	{
	    tprintf("shutting down server #%d...", i);
	    obj->shutdown();
	    tprintf("ok\n");
	}
	else if(j == 1 || i + 1 > ports.size())
	{
	    tprintf("aborting server #%d...", i);
	    try
	    {
	        obj->abort();
	        test(false);
	    }
	    catch(const IceE::ConnectionLostException&)
	    {
	        tprintf("ok\n");
	    }
	    catch(const IceE::ConnectFailedException&)
	    {
	        tprintf("ok\n");
	    }
	}
	else if(j == 2)
	{
	    tprintf("aborting server #%d and #%d with idempotent call...", i, i + 1);
	    try
	    {
	        obj->idempotentAbort();
	        test(false);
	    }
	    catch(const IceE::ConnectionLostException&)
	    {
	        tprintf("ok\n");
	    }
	    catch(const IceE::ConnectFailedException&)
	    {
	        tprintf("ok\n");
	    }

	    ++i;
	}
	else if(j == 3)
	{
	    tprintf("aborting server #%d and #%d with nonmutating call...", i, i + 1);
	    try
	    {
	        obj->nonmutatingAbort();
	        test(false);
	    }
	    catch(const IceE::ConnectionLostException&)
	    {
	        tprintf("ok\n");
	    }
	    catch(const IceE::ConnectFailedException&)
	    {
	        tprintf("ok\n");
	    }

	    ++i;
	}
	else
	{
	    assert(false);
	}
    }

    tprintf("testing whether all servers are gone...");
    try
    {
	obj->ice_ping();
	test(false);
    }
    catch(const IceE::LocalException&)
    {
	tprintf("ok\n");
    }
}
