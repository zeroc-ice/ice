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
#include <StringConverterI.h>

using namespace std;

DEFINE_TEST("client")

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    Test::TestIntfPrxPtr allTests(const Ice::CommunicatorPtr&);
    Test::TestIntfPrxPtr test = allTests(communicator);
    test->shutdown();

    return EXIT_SUCCESS;
}

int
main(int argc, char** argv)
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
    Ice::registerIceWS(true);
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
        cerr << ex << ": " << ex.ice_stackTrace() << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;

}
