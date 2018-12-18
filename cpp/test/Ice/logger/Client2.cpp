// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>

using namespace std;

class Client2 : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client2::run(int argc, char** argv)
{
#ifdef _WIN32
    int cp = GetConsoleOutputCP();
    //
    // 28605 == ISO 8859-15 codepage
    //
    SetConsoleOutputCP(28605);
    setProcessStringConverter(Ice::createWindowsStringConverter(1250));
#else
    setProcessStringConverter(Ice::createIconvStringConverter<char>("ISO8859-15"));
#endif

    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->load("config.client");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getLogger()->trace("info", "XXX");

#ifdef _WIN32
    SetConsoleOutputCP(cp);
#endif
}

DEFINE_TEST(Client2)
