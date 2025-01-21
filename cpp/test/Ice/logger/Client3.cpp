// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"

#ifdef _WIN32
#    include <windows.h>
#endif

using namespace std;

class Client3 : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Client3::run(int argc, char** argv)
{
#ifdef _WIN32
    int cp = GetConsoleOutputCP();
    SetConsoleOutputCP(CP_UTF8);
#endif
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->load("config.client");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getLogger()->trace("info", "XXX");
#ifdef _WIN32
    SetConsoleOutputCP(cp);
#endif
}

DEFINE_TEST(Client3)
