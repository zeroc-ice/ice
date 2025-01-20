// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestAMDI.h"
#include "TestHelper.h"

using namespace std;

class ServerAMD : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
ServerAMD::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Admin.Endpoints", "tcp");
    properties->setProperty("Ice.Admin.InstanceName", "server");
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.Warn.Dispatch", "0");
    properties->setProperty("Ice.MessageSizeMax", "50000");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(make_shared<MetricsI>(), Ice::stringToIdentity("metrics"));
    adapter->activate();

    communicator->getProperties()->setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
    Ice::ObjectAdapterPtr forwardingAdapter = communicator->createObjectAdapter("ForwardingAdapter");
    forwardingAdapter->addDefaultServant(adapter->dispatchPipeline(), "");
    forwardingAdapter->activate();

    communicator->getProperties()->setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
    Ice::ObjectAdapterPtr controllerAdapter = communicator->createObjectAdapter("ControllerAdapter");
    controllerAdapter->add(make_shared<ControllerI>(adapter), Ice::stringToIdentity("controller"));
    controllerAdapter->activate();

    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(ServerAMD)
