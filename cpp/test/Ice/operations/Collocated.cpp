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

class Collocated : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Collocated::run(int argc, char** argv)
{
    Ice::PropertiesPtr properties = createTestProperties(argc, argv);
    properties->setProperty("Ice.BatchAutoFlushSize", "100");
    Ice::CommunicatorHolder communicator = initialize(argc, argv, properties);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    communicator->getProperties()->setProperty("TestAdapter.AdapterId", "test");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPrxPtr prx = adapter->add(ICE_MAKE_SHARED(MyDerivedClassI), Ice::stringToIdentity("test"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    test(!prx->ice_getConnection());

    Test::MyClassPrxPtr allTests(Test::TestHelper*);
    allTests(this);
}

DEFINE_TEST(Collocated)
