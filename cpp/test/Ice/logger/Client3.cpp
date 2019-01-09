// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace std;

class Client3 : public Test::TestHelper
{
public:

    void run(int, char**);
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
