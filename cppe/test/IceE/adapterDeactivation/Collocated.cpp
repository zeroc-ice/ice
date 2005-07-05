// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace IceE;
using namespace Test;

int
run(int argc, char* argv[], const IceE::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
    IceE::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->activate();

    TestIntfPrx allTests(const CommunicatorPtr&);
    allTests(communicator);

    adapter->waitForDeactivate();
    return EXIT_SUCCESS;
}

main(int argc, char* argv[])
{
    int status;
    IceE::CommunicatorPtr communicator;

    try
    {
        communicator = IceE::initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const IceE::Exception& ex)
    {
        fprintf(stderr, "%s\n", ex.toString().c_str());
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(const IceE::Exception& ex)
        {
            fprintf(stderr, "%s\n", ex.toString().c_str());
            status = EXIT_FAILURE;
        }
    }

    return status;
}
