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
#include <TestI.h>
#include <WstringI.h>
#include <StringConverterI.h>

using namespace std;

DEFINE_TEST("collocated")

int
run(int, char**, const Ice::CommunicatorPtr& communicator)
{
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->add(new TestIntfI(communicator), communicator->stringToIdentity("test"));
    adapter->add(new Test1::WstringClassI, communicator->stringToIdentity("wstring1"));
    adapter->add(new Test2::WstringClassI, communicator->stringToIdentity("wstring2"));

    Test::TestIntfPrx allTests(const Ice::CommunicatorPtr&);
    allTests(communicator);

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
        IceUtil::setProcessStringConverter(new Test::StringConverterI());
        IceUtil::setProcessWstringConverter(new Test::WstringConverterI());

        Ice::InitializationData initData;
        initData.properties = Ice::createProperties(argc, argv);
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010");
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

    return status;
}
