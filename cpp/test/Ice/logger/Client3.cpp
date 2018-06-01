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
