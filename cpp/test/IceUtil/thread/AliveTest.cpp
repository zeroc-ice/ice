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

class Synchronizer : public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    Synchronizer() : _done(false)
    {
    }

    void p()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
	while (!_done)
	{
	    wait();
	}
    }

    void v()
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
    AliveTestThread(Synchronizer& child, Synchronizer& parent) : _child(child), _parent(parent)
    {
    }

    virtual void run()
    {
	try
	{
	   _child.v();
	   _parent.p();
	}
	catch(IceUtil::ThreadLockedException &)
	{
	}
    }

private:
    Synchronizer& _child;
    Synchronizer& _parent;
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
    Synchronizer parentReady;
    Synchronizer childReady;
    AliveTestThreadPtr t = new AliveTestThread(childReady, parentReady);
    IceUtil::ThreadControl c = t->start();
    childReady.p();
    test(c.isAlive());
    parentReady.v();
    c.join();
    test(!c.isAlive());
}
