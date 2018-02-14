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

void
usage(const char* n)
{
    cerr << "Usage: " << n << " port...\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    vector<int> ports;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        ports.push_back(atoi(argv[i]));
    }

    if(ports.empty())
    {
        cerr << argv[0] << ": no ports specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    try
    {
        void allTests(const Ice::CommunicatorPtr&, const vector<int>&);
        allTests(communicator, ports);
    }
    catch(const Ice::Exception& ex)
    {
        cout << ex << endl;
        test(false);
    }

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
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);

        //
        // This test aborts servers, so we don't want warnings.
        //
        initData.properties->setProperty("Ice.Warn.Connections", "0");

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
