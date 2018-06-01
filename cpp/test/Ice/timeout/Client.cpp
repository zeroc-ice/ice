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

    //
    // For this test, we want to disable retries.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");

#if TARGET_OS_IPHONE != 0
    //
    // COMPILERFIX: Disable connect timeout introduced for
    // workaround to iOS device hangs when using SSL
    //
    properties->setProperty("Ice.Override.ConnectTimeout", "");
#endif

    //
    // This test kills connections, so we don't want warnings.
    //
    properties->setProperty("Ice.Warn.Connections", "0");

    //
    // Limit the send buffer size, this test relies on the socket
    // send() blocking after sending a given amount of data.
    //
    properties->setProperty("Ice.TCP.SndSize", "50000");

    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);

    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
