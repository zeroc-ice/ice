// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <CreateTest.h>
#include <TestHelper.h>

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
    }
}
