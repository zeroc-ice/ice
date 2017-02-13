// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Controller.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Test;

int
run(int argc, char* argv[], const CommunicatorPtr& communicator)
{
    if(argc < 2)
    {
        cerr << "Usage: " << argv[0] << " proxy" << endl;
        return EXIT_FAILURE;
    }

    ControllerPrx control = ControllerPrx::uncheckedCast(communicator->stringToProxy(argv[1]));
    control->stop();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    CommunicatorPtr communicator;
    InitializationData initData = getTestInitData(argc, argv);
    try
    {
        communicator = initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Exception& ex)
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
