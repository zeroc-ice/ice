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

#include <IceUtil/IceUtil.h>

#include <stdio.h>

#include <AliveTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread alive");

class CondVar : public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    CondVar() : _done(false)
    {
    }

    void wait()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	while (!_done)
	{
	    wait();
	}
    }

    void signal()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
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
	   _parentReady.wait();
	}
	catch(IceUtil::ThreadLockedException &)
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
    IceUtil::ThreadControl c = t->start();
    childCreated.wait();
    test(c.isAlive());
    parentReady.signal();
    c.join();
    test(!c.isAlive());
}
