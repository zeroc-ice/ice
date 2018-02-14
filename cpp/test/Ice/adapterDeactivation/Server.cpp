// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <ServantLocatorI.h>
#include <TestCommon.h>

DEFINE_TEST("server")

using namespace std;
using namespace Ice;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12010:udp");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    ServantLocatorPtr locator = new ServantLocatorI;
    adapter->addServantLocator(locator, "");
    adapter->activate();
    TEST_READY
    adapter->waitForDeactivate();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        communicator = Ice::initialize(argc, argv);
        status = run(argc, argv, communicator);
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
