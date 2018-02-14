// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

int
run(int, char**, const Ice::CommunicatorPtr& comm)
{
    void allTests(const Ice::CommunicatorPtr&);
    allTests(comm);

    //
    // Shutdown the IceBox server.
    //
    Ice::ProcessPrx::uncheckedCast(comm->stringToProxy("DemoIceBox/admin -f Process:default -p 9996"))->shutdown();

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
        initData.properties->setProperty("Ice.Default.Host", "127.0.0.1");
        communicator = Ice::initialize(argc, argv, initData);
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
