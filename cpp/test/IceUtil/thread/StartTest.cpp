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

#include <StartTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread start");

class StartTestThread : public Thread
{
public:
    
    StartTestThread()
    {
    }

    virtual void run()
    {
    }
};

typedef Handle<StartTestThread> StartTestThreadPtr;

StartTest::StartTest() :
    TestBase(createTestName)
{
}

void
StartTest::run()
{
    //
    // Check that calling start() more than once raises ThreadStartedException.
    //
    StartTestThreadPtr t = new StartTestThread();
    ThreadControl control = t->start();
    control.join();
    bool gotException = false;
    try {
	t->start();
    }
    catch(const ThreadStartedException&)
    {
	gotException = true;
    }
    test(gotException);
}
