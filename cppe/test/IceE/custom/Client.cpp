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

class CustomTestApplication : public TestApplication
{
public:

    CustomTestApplication() :
        TestApplication("custom client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();

	loadConfig(initData.properties);
	initData.logger = getLogger();
	setCommunicator(Ice::initialize(argc, argv, initData));

        Test::TestIntfPrx allTests(const Ice::CommunicatorPtr&);
        Test::TestIntfPrx test = allTests(communicator());
        test->shutdown();

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    CustomTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    CustomTestApplication app;
    return app.main(argc, argv);
}

#endif
