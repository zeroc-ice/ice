// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace std;

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    void allTests(const Ice::CommunicatorPtr&);
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
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        communicator = Ice::initialize(argc, argv, initData);
        communicator->getProperties()->parseCommandLineOptions("", Ice::argsToStringSeq(argc, argv));
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
