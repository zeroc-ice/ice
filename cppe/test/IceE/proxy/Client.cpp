// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <TestApplication.h>
#include <Test.h>

using namespace std;

class ProxyTestApplication : public TestApplication
{
public:

    ProxyTestApplication() :
        TestApplication("proxy client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();

        //
        // In this test, we need at least two threads in the
        // client side thread pool for nested AMI.
        //
        initData.properties->setProperty("Ice.ThreadPool.Client.Size", "2");
        initData.properties->setProperty("Ice.ThreadPool.Client.SizeWarn", "0");

	loadConfig(initData.properties);

	//
	// Now parse argc/argv into initData
	//
	initData.properties = Ice::createProperties(argc, argv, initData.properties); 

	initData.logger = getLogger();	
	setCommunicator(Ice::initialize(argc, argv, initData));
	
        //
        // We don't want connection warnings because of the timeout test.
        //
        communicator()->getProperties()->setProperty("Ice.Warn.Connections", "0");

        //
        // Use a faster connection monitor timeout to test AMI
        // timeouts.
        //
        communicator()->getProperties()->setProperty("Ice.MonitorConnections", "1");

        Test::MyClassPrx allTests(const Ice::CommunicatorPtr&);
        Test::MyClassPrx myClass = allTests(communicator());

        myClass->shutdown();

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    ProxyTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    ProxyTestApplication app;
    return app.main(argc, argv);
}

#endif
