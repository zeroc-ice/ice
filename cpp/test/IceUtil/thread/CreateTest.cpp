// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
