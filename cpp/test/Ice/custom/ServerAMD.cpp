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
#include <TestAMDI.h>
#include <WstringAMDI.h>
#include <StringConverterI.h>

using namespace std;

DEFINE_TEST("serveramd")

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0));
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(ICE_MAKE_SHARED(TestIntfI,communicator), Ice::stringToIdentity("TEST"));
    adapter->add(ICE_MAKE_SHARED(Test1::WstringClassI), Ice::stringToIdentity("WSTRING1"));
    adapter->add(ICE_MAKE_SHARED(Test2::WstringClassI), Ice::stringToIdentity("WSTRING2"));

    adapter->activate();
    TEST_READY
    communicator->waitForShutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        setProcessStringConverter(ICE_MAKE_SHARED(Test::StringConverterI));
        setProcessWstringConverter(ICE_MAKE_SHARED(Test::WstringConverterI));

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
