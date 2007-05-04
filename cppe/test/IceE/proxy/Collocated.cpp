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
#include <TestI.h>

using namespace std;

class ProxyTestApplication : public TestApplication
{
public:

    ProxyTestApplication() :
        TestApplication("proxy collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
    	Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");

	loadConfig(initData.properties);
	//
	// Now parse argc/argv into initData.properties
	//
	initData.properties = Ice::createProperties(argc, argv, initData.properties);
	initData.logger = getLogger();
	setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        adapter->add(new MyDerivedClassI, communicator()->stringToIdentity("test"));
        adapter->activate();
        
        Test::MyClassPrx allTests(const Ice::CommunicatorPtr&);
        allTests(communicator());

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
