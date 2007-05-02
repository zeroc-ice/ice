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
#include <WstringI.h>

using namespace std;

class CustomTestApplication : public TestApplication
{
public:

    CustomTestApplication() :
        TestApplication("custom collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();

	initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
	//initData.properties->setProperty("Ice.Trace.Network", "5");
	//initData.properties->setProperty("Ice.Trace.Protocol", "5");

	loadConfig(initData.properties);
	initData.logger = getLogger();
	setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        adapter->add(new TestIntfI(communicator()), communicator()->stringToIdentity("test"));
#ifdef ICEE_HAS_WSTRING
	adapter->add(new Test1::WstringClassI, communicator()->stringToIdentity("wstring1"));
	adapter->add(new Test2::WstringClassI, communicator()->stringToIdentity("wstring2"));
#endif
        adapter->activate();

        Test::TestIntfPrx allTests(const Ice::CommunicatorPtr&);
        allTests(communicator());

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
