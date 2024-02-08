//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/IceUtil.h>

#include <CreateTest.h>
#include <TestHelper.h>

using namespace std;
using namespace IceUtil;

static const string createTestName("thread create");

class CreateTestThread final : public Thread
{
public:

    CreateTestThread() :
        threadran(false)
    {
    }

    void run() final
    {
        threadran = true;
    }

    bool threadran;
};

CreateTest::CreateTest() :
    TestBase(createTestName)
{
}

void
CreateTest::run()
{
    for(int i = 0; i < 4096 ; ++i)
    {
        auto t = make_shared<CreateTestThread>();
        ThreadControl control = t->start();
        control.join();
        test(t->threadran);
    }
}
