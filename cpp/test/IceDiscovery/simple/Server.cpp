// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "TestHelper.h"
#include "TestI.h"

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::InitializationData initData;
    initData.properties = createTestProperties(argc, argv);

    if (IceInternal::isMinBuild())
    {
        // We can provide udpPluginFactory() here, but it's optional even with a static build since we use TestHelper's
        // initialize with registerPlugins = true (the default).
        initData.pluginFactories = {Ice::udpPluginFactory(), IceDiscovery::discoveryPluginFactory()};
    }

    Ice::CommunicatorHolder communicator = initialize(argc, argv, std::move(initData));
    Ice::PropertiesPtr properties = communicator->getProperties();

    int num = argc == 2 ? stoi(argv[1]) : 0;

    properties->setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));

    {
        ostringstream os;
        os << "control" << num;
        properties->setProperty("ControlAdapter.AdapterId", os.str());
    }
    properties->setProperty("ControlAdapter.ThreadPool.Size", "1");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ControlAdapter");
    {
        ostringstream os;
        os << "controller" << num;
        adapter->add(std::make_shared<ControllerI>(), Ice::stringToIdentity(os.str()));
    }
    adapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
