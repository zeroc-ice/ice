// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <HelloI.h>

using namespace std;

static Ice::CommunicatorPtr communicator;

static void 
onCtrlC(int)
{
    if(communicator)
    {
        communicator->shutdown();
    }
}

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;

    IceUtil::CtrlCHandler ctrCHandler(onCtrlC);

    try
    {
        communicator = Ice::initialize(argc, argv);
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("Hello", "tcp -p 10000");
        adapter->add(new HelloI, communicator->stringToIdentity("hello"));
        adapter->activate();
        communicator->waitForShutdown();
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
