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
#include <unistd.h>

#include <AliveTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread alive");

class AliveTestThread : public Thread
{
public:
    
    AliveTestThread(IceUtil::RWRecMutex& m) : _m(m)
    {
    }

    virtual void run()
    {
	try
	{
	    IceUtil::RWRecMutex::TryWLock lock(_m, IceUtil::Time::seconds(1));
	}
	catch(IceUtil::ThreadLockedException &)
	{
	}
    }

private:
    RWRecMutex& _m;
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
    IceUtil::RWRecMutex m;
    m.writelock();
    AliveTestThreadPtr t = new AliveTestThread(m);
    IceUtil::ThreadControl c = t->start();
    test(c.isAlive());
    c.join();
    test(!c.isAlive());
}
