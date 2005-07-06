// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <AliveTest.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;

static const string createTestName("thread alive");

class CondVar : public Ice::Monitor<Ice::RecMutex>
{
public:

    CondVar() :
	_done(false)
    {
    }

    void waitForSignal()
    {
	Ice::Monitor<Ice::RecMutex>::Lock lock(*this);
	while(!_done)
	{
	    wait();
	}
    }

    void signal()
    {
	Ice::Monitor<Ice::RecMutex>::Lock lock(*this);
	_done = true;
	notify();
    }

private:

    bool _done;
};

class AliveTestThread : public Thread
{
public:

    AliveTestThread(CondVar& childCreated, CondVar& parentReady) :
	_childCreated(childCreated), _parentReady(parentReady)
    {
    }

    virtual void run()
    {
	try
	{
	   _childCreated.signal();
	   _parentReady.waitForSignal();
	}
	catch(Ice::ThreadLockedException &)
	{
	}
    }

private:

    CondVar& _childCreated;
    CondVar& _parentReady;
};

typedef Handle<AliveTestThread> AliveTestThreadPtr;

AliveTest::AliveTest() :
    TestBase(createTestName)
{
}

void
AliveTest::run()
{
    //
    // Check that calling isAlive() returns the correct result for alive and
    // and dead threads.
    //
    CondVar childCreated;
    CondVar parentReady;
    AliveTestThreadPtr t = new AliveTestThread(childCreated, parentReady);
    Ice::ThreadControl c = t->start();
    childCreated.waitForSignal();
    test(c.isAlive());
    parentReady.signal();
    c.join();
    test(!c.isAlive());
}
