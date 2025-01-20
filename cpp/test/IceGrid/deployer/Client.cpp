// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

class Client : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    bool withTarget = false;
    if (argc > 1)
    {
        int i = 1;
        while (i < argc)
        {
            if (strcmp(argv[i], "-t") == 0)
            {
                withTarget = true;
                break;
            }
            i++;
        }
    }

    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = communicator->getProperties()->parseCommandLineOptions("", args);
    Ice::stringSeqToArgs(args, argc, argv);

    if (!withTarget)
    {
        void allTests(Test::TestHelper*);
        allTests(this);
    }
    else
    {
        void allTestsWithTarget(Test::TestHelper*);
        allTestsWithTarget(this);
    }
}

DEFINE_TEST(Client)
