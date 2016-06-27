// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/IceSSL.h>
#include <TestCommon.h>
#include <Test.h>

DEFINE_TEST("client")

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string testdir;
#if TARGET_OS_IPHONE == 0
    if(argc < 2)
    {
        cerr << "Usage: " << argv[0] << " testdir" << endl;
        return 1;
    }
    testdir = argv[1];
#endif

    Test::ServerFactoryPrxPtr allTests(const Ice::CommunicatorPtr&, const string&, bool);

    try
    {
        cerr << "testing with PKCS12 certificates..." << endl;
        Test::ServerFactoryPrxPtr factory = allTests(communicator, testdir, true);
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
        cerr << "testing with PEM certificates..." << endl;
        factory = allTests(communicator, testdir, false);
#endif
        if(factory)
        {
            factory->shutdown();
        }
    }
    catch(const IceSSL::CertificateReadException& ex)
    {
        cout << "couldn't read certificate: " << ex.reason << endl;
    }
    catch(const std::exception& ex)
    {
        cout << "unexpected exception: " << ex.what() << endl;
    }
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
