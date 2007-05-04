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

class OperationsTestApplication : public TestApplication
{
public:

    OperationsTestApplication() :
        TestApplication("operations collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
    	Ice::InitializationData initData;
	initData.properties = Ice::createProperties();

	//
	// We must set MessageSizeMax to an explicit values, because
	// we run tests to check whether Ice.MemoryLimitException is
	// raised as expected.
	//
	initData.properties->setProperty("Ice.MessageSizeMax", "100");
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

        Test::MyClassPrx allTests(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
        allTests(communicator(), initData);

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    OperationsTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    OperationsTestApplication app;
    return app.main(argc, argv);
}

#endif
