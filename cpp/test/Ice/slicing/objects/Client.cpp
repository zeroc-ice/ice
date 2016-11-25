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
#include <ClientPrivate.h>

using namespace std;
using namespace Test;

DEFINE_TEST("client")

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    TestIntfPrxPtr allTests(const Ice::CommunicatorPtr&);
    TestIntfPrxPtr Test = allTests(communicator);
    Test->shutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#   if defined(__linux)
    Ice::registerIceBT();
#   endif
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        //
        // For this test, we enable object collection.
        //
        initData.properties->setProperty("Ice.CollectObjects", "1");

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        RemoteConfig rc("Ice/slicing/objects", argc, argv, ich.communicator());
        int status = run(argc, argv, ich.communicator());
        rc.finished(status);
        return status;
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
