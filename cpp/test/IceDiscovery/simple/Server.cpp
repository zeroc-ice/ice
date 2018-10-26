// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h>

using namespace std;

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceDiscovery();
#endif

    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    Ice::PropertiesPtr properties = communicator->getProperties();

    int num = argc == 2 ? atoi(argv[1]) : 0;

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
        adapter->add(ICE_MAKE_SHARED(ControllerI), Ice::stringToIdentity(os.str()));
    }
    adapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
