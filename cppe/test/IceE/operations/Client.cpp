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

class OperationsTestApplication : public TestApplication
{
public:

    OperationsTestApplication() :
        TestApplication("operations client")
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

        Test::MyClassPrx allTests(const Ice::CommunicatorPtr&, const Ice::InitializationData&);
        Test::MyClassPrx myClass = allTests(communicator(), initData);

        tprintf("testing server shutdown... ");
        myClass->shutdown();
        try
        {
	    myClass->opVoid();
	    test(false);
        }
        catch(const Ice::LocalException&)
        {
            tprintf("ok\n");
        }

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
