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
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
    void allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Client)
