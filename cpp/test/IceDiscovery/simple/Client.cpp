// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

// Link with IceDiscovery on Windows.
#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceDiscovery"))
#endif

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
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);

    if (IceInternal::isMinBuild())
    {
        initData.pluginFactories = {Ice::discoveryPluginFactory()};
    }

    Ice::CommunicatorHolder communicator = initialize(argc, argv, std::move(initData));
    int num = argc == 2 ? stoi(argv[1]) : 1;

    void allTests(Test::TestHelper*, int);
    allTests(this, num);
}

DEFINE_TEST(Client)
