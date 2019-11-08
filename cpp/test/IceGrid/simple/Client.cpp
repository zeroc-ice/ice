//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class Client final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Client::run(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceLocatorDiscovery(false);
#endif
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);

    bool withDeploy = false;

    for(int i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "--with-deploy") == 0)
        {
            withDeploy = true;
            break;
        }
    }

    if(!withDeploy)
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
