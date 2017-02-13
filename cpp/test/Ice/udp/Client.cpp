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
        os << "control:" << getTestEndpoint(communicator, i, "tcp");
        ICE_UNCHECKED_CAST(TestIntfPrx, communicator->stringToProxy(os.str()))->shutdown();
    }
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        initData.properties->setProperty("Ice.Warn.Connections", "0");
        initData.properties->setProperty("Ice.UDP.RcvSize", "16384");
        initData.properties->setProperty("Ice.UDP.SndSize", "16384");

        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);
        return run(argc, argv, ich.communicator());
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return  EXIT_FAILURE;
    }
}
