// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#include <stdio.h>

#include <StartTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread start");

static IceUtil::Mutex threadCountMutex;
static int threadCount = 0;

class StartTestThread : public Thread
{
public:
    
    StartTestThread()
    {
    }

    virtual void run()
    {
	IceUtil::Mutex::Lock sync(threadCountMutex);
	--threadCount;
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
    try
    {
	t->start();
	test(false);
    }
    catch(const ThreadStartedException&)
    {
    }

    threadCount = 0;
    //
    // Now let's create a bunch of short-lived threads
    //
#ifdef _WIN32_WCE
    for(int i = 0; i < 50; i++)
    {
	for(int j = 0; j < 5; j++)
	{
	    {
		IceUtil::Mutex::Lock sync(threadCountMutex);
		++threadCount;
	    }
	    Thread* t = new StartTestThread;
	    t->start().detach();
	}

	// Wait for the threads to all terminate. I don't want to use
	// a monitor here, since monitor hasn't been tested yet.
	while(true)
	{
	    {
		IceUtil::Mutex::Lock sync(threadCountMutex);
		if(threadCount == 0)
		{
		    break;
		}
	    }
	    ThreadControl::sleep(Time::milliSeconds(5));
	}
    }
#else
    for(int i = 0; i < 50; i++)
    {
	for(int j = 0; j < 50; j++)
	{
	    Thread* t = new StartTestThread;
	    t->start().detach();
	}
	ThreadControl::sleep(Time::milliSeconds(5));
    }
#endif
}
