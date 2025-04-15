// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);
    bool withDeploy = false;

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--with-deploy") == 0)
        {
            withDeploy = true;
            break;
        }
    }

    if (!withDeploy)
    {
        void allTests(Test::TestHelper*);
        allTests(this);
    }
    else
    {
        void allTestsWithDeploy(Test::TestHelper*);
        allTestsWithDeploy(this);
    }
}

DEFINE_TEST(Client)
