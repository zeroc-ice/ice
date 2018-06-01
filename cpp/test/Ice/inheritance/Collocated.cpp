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
#include <TestI.h>

using namespace std;
using namespace Test;

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(InitialI, adapter);
    adapter->add(object, Ice::stringToIdentity("initial"));
    InitialPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
