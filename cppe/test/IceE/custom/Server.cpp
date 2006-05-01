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
#include <TestI.h>
#include <WstringI.h>
#include <StringConverterI.h>

using namespace std;

class CustomTestApplication : public TestApplication
{
public:

    CustomTestApplication() :
        TestApplication("custom server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
	initData.stringConverter = new Test::StringConverterI();
	initData.wstringConverter = new Test::WstringConverterI();

	initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
	//initData.properties->setProperty("Ice.Trace.Network", "5");
	//initData.properties->setProperty("Ice.Trace.Protocol", "5");

	loadConfig(initData.properties);
	initData.logger = getLogger();
	setCommunicator(Ice::initialize(argc, argv, initData));
	
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        adapter->add(new TestIntfI(communicator()), Ice::stringToIdentity("test"));
	adapter->add(new Test1::WstringClassI, Ice::stringToIdentity("wstring1"));
	adapter->add(new Test2::WstringClassI, Ice::stringToIdentity("wstring2"));
	adapter->activate();

#ifndef _WIN32_WCE
        communicator()->waitForShutdown();
#endif

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
