// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    if(argc < 2)
    {
        cerr << "Usage: " << argv[0] << " testdir" << endl;
        return 1;
    }

    void allTests(const Ice::CommunicatorPtr&, const string&, bool, bool);

    cerr << "testing with PKCS12 certificates..." << endl;
    allTests(communicator, argv[1], true, false);
    cerr << "testing with PEM certificates..." << endl;
    allTests(communicator, argv[1], false, true);

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    //
    // Explicitly register the IceSSL plugin to test registerIceSSL. The tests
    // don't set Ice.Plugin.IceSSL to ensure the plugin is registered without
    // the property setting.
    //
    Ice::registerIceSSL();

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
