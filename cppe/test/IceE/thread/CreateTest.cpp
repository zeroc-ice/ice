// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/DisableWarnings.h>
#include <IceE/IceE.h>

#include <stdio.h>

#include <CreateTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread create");

class CreateTestThread : public Thread
{
public:
    
    CreateTestThread() :
	threadran(false)
    {
    }

    virtual void run()
    {
	threadran = true;
    }

    bool threadran;
};

typedef Handle<CreateTestThread> CreateTestThreadPtr;

CreateTest::CreateTest() :
    TestBase(createTestName)
{
}

void
CreateTest::run()
{
    int nthreads = 4096;
#ifdef _WIN32_WCE
    //
    // 4096 threads takes forever to start on the WinCe emulator.
    //
    nthreads = 512;
#endif
    for(int i = 0; i < nthreads ; ++i)
    {
	CreateTestThreadPtr t = new CreateTestThread();
	ThreadControl control = t->start();
	control.join();
	test(t->threadran);
#ifdef _WIN32_WCE
	if((i % 32) == 0)
	{
	    tprintf(".");
	}
#else
	if((i % 256) == 0)
	{
	    char buf[5];
	    sprintf(buf, "%04d", i);
	    tprintf("%s", buf);
	}
#endif
    }
#ifndef _WIN32_WCE
    tprintf("    ");
#endif
}
