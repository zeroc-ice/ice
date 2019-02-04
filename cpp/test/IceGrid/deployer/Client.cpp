//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    bool withTarget = false;
    if(argc > 1)
    {
        int i = 1;
        while(i < argc)
        {
            if(strcmp(argv[i], "-t") == 0)
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

    if(!withTarget)
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
