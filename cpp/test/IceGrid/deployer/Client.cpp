// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    bool withTarget = false;
    if(argc > 1)
    {
        int i = 1;
        while(i < argc)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                withTarget = true;
                break;
            }
            i++;
        }
    }

    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = communicator->getProperties()->parseCommandLineOptions("", args);
    Ice::stringSeqToArgs(args, argc, argv);

    if(!withTarget)
    {
        void allTests(const Ice::CommunicatorPtr&);
        allTests(communicator);
    }
    else
    {
        void allTestsWithTarget(const Ice::CommunicatorPtr&);
        allTestsWithTarget(communicator);
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
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
