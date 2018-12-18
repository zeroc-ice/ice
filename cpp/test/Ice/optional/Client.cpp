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
#include <TestI.h>

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
#ifndef ICE_CPP11_MAPPING
    properties->setProperty("Ice.CollectObjects", "1");
#endif
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    InitialPrxPtr allTests(Test::TestHelper*, bool);
    InitialPrxPtr initial = allTests(this, false);
    initial->shutdown();
}

DEFINE_TEST(Client)
