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

DEFINE_TEST("client")

using namespace std;
using namespace Test;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    void allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);

    int num = argc == 2 ? atoi(argv[1]) : 1;
    for(int i = 0; i < num; i++)
    {
        ostringstream os;
        os << "control:tcp -p " << (12010 + i);
        TestIntfPrx::uncheckedCast(communicator->stringToProxy(os.str()))->shutdown();
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

        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties->setProperty("Ice.UDP.SndSize", "16384");

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

#if TARGET_OS_IPHONE != 0
    //
    // iOS WORKAROUND: without a sleep before the communicator
    // destroy, the close on the UDP socket hangs.
    //
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
#endif

    return status;
}
