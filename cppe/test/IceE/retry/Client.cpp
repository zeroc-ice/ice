// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class RetryTestApplication : public TestApplication
{
public:

    RetryTestApplication() :
        TestApplication("retry client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::PropertiesPtr properties = Ice::createProperties();
	//properties->setProperty("Ice.Trace.Network", "5");
	//properties->setProperty("Ice.Trace.Protocol", "5");

	loadConfig(properties);

	//
	// For this test, we want to disable retries.
	//
	properties->setProperty("Ice.RetryIntervals", "-1");

	setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

	//
	// This test kills connections, so we don't want warnings.
	//
	properties->setProperty("Ice.Warn.Connections", "0");

        Test::RetryPrx allTests(const Ice::CommunicatorPtr&);
        Test::RetryPrx retry = allTests(communicator());
        retry->shutdown();
        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    RetryTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    RetryTestApplication app;
    return app.main(argc, argv);
}

#endif
