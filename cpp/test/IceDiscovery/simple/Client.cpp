// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "Test.h"
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
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);

    if (IceInternal::isMinBuild())
    {
        initData.pluginFactories = {IceDiscovery::discoveryPluginFactory()};
    }

    Ice::CommunicatorHolder communicator{initialize(std::move(initData))};
    int num = argc == 2 ? stoi(argv[1]) : 1;

    void allTests(Test::TestHelper*, int);
    allTests(this, num);
}

DEFINE_TEST(Client)
