// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <TestI.h>

DEFINE_TEST("collocated")

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator,
    const Ice::InitializationData&)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010");
    communicator->getProperties()->setProperty("TestAdapter.AdapterId", "test");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPrx prx = adapter->add(new MyDerivedClassI, communicator->stringToIdentity("test"));
    //adapter->activate(); // Don't activate OA to ensure collocation is used.

    test(!prx->ice_getConnection());

    Test::MyClassPrx allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);

        //
        // We must set MessageSizeMax to an explicit values, because
        // we run tests to check whether Ice.MemoryLimitException is
        // raised as expected.
        //
        initData.properties->setProperty("Ice.MessageSizeMax", "100");

        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator, initData);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}
