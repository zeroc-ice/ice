// **********************************************************************
//
// Copyright (c) 2002
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
    for(int i = 0; i < 4096 ; ++i)
    {
	CreateTestThreadPtr t = new CreateTestThread();
	ThreadControl control = t->start();
	control.join();
	test(t->threadran);
	if((i % 256) == 0)
	{
	    char buf[5];
	    sprintf(buf, "%04d", i);
	    cout << buf << "" << flush;
	}
    }
    cout << "    " << flush;
}
