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
#include <TestI.h>

DEFINE_TEST("server")

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    int num = argc == 2 ? atoi(argv[1]) : 0;

    {
        ostringstream os;
        os << getTestEndpoint(communicator, num);
        cerr << os.str() << endl;
        properties->setProperty("ControlAdapter.Endpoints", os.str());
    }
    {
        ostringstream os;
        os << "control" << num;
        properties->setProperty("ControlAdapter.AdapterId", os.str());
    }
    properties->setProperty("ControlAdapter.ThreadPool.Size", "1");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("ControlAdapter");
    {
        ostringstream os;
        os << "controller" << num;
        adapter->add(ICE_MAKE_SHARED(ControllerI), Ice::stringToIdentity(os.str()));
    }
    adapter->activate();

    TEST_READY

    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
    Ice::registerIceDiscovery();
#endif

    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
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
        communicator->destroy();
    }

    return status;
}
