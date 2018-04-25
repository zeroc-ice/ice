// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>
#include <TestCommon.h>

using namespace std;

void
usage(const char* n)
{
    cerr << "Usage: " << n << " port\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    int port = 0;
    for(int i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            cerr << argv[0] << ": unknown option `" << argv[i] << "'" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        if(port > 0)
        {
            cerr << argv[0] << ": only one port can be specified" << endl;
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        port = atoi(argv[i]);
    }

    if(port <= 0)
    {
        cerr << argv[0] << ": no port specified" << endl;
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    ostringstream endpts;
    endpts << getTestEndpoint(communicator, port);
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", endpts.str());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    Ice::ObjectPtr object = ICE_MAKE_SHARED(TestI);
    adapter->add(object, Ice::stringToIdentity("test"));
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
    Ice::registerIceUDP(true);
#endif

    try
    {
        //
        // In this test, we need a longer server idle time, otherwise
        // our test servers may time out before they are used in the
        // test.
        //
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.ServerIdleTime", "120"); // Two minutes.

        Ice::CommunicatorHolder ich(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
