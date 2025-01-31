// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h"

#include <stdexcept>

using namespace std;

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);

    int port = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            ostringstream os;
            os << ": unknown option `" << argv[i] << "'";
            throw invalid_argument(os.str());
        }

        if (port > 0)
        {
            throw runtime_error("only one port can be specified");
        }

        port = stoi(argv[i]);
    }

    if (port <= 0)
    {
        throw runtime_error("no port specified");
    }

    ostringstream endpts;
    endpts << getTestEndpoint(port);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpts.str());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = std::make_shared<TestI>();
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
    serverReady();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
