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
    Ice::registerIceSSL();
#endif
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        IceUtil::setProcessStringConverter(ICE_MAKE_SHARED(Test::StringConverterI));
        IceUtil::setProcessWstringConverter(ICE_MAKE_SHARED(Test::WstringConverterI));

        communicator = Ice::initialize(argc, argv);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << ": " << ex.ice_stackTrace() << endl;
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
