// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <ClientPrivate.h>

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
    //
    // For this test, we enable object collection.
    //
    properties->setProperty("Ice.CollectObjects", "1");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    TestIntfPrxPtr allTests(Test::TestHelper*);
    TestIntfPrxPtr prx = allTests(this);
    prx->shutdown();
}

DEFINE_TEST(Client)
