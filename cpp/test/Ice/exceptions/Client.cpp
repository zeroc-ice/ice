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
#include <Test.h>

using namespace std;
using namespace Test;

class Client : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Client::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("Ice.MessageSizeMax", "10"); // 10KB max
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    ThrowerPrxPtr allTests(Test::TestHelper*);
    ThrowerPrxPtr thrower = allTests(this);
    thrower->shutdown();
}

DEFINE_TEST(Client)
