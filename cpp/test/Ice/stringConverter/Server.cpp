// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

DEFINE_TEST("server");

using namespace std;

//
// Server side is pure unicode
//
class MyObjectI : public Test::MyObject
{
public:

    virtual wstring widen(ICE_IN(string) msg, const Ice::Current&)
    {
        return stringToWstring(msg, Ice::getProcessStringConverter(),
                               Ice::getProcessWstringConverter());
    }

    virtual string narrow(ICE_IN(wstring) wmsg, const Ice::Current&)
    {
        return wstringToString(wmsg, Ice::getProcessStringConverter(),
                               Ice::getProcessWstringConverter());
    }

    virtual void shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }
};

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(MyObjectI), Ice::stringToIdentity("test"));
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
