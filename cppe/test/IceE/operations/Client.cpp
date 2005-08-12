// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	Ice::PropertiesPtr properties = Ice::createProperties();
	//
	// We must set MessageSizeMax to an explicit values, because
	// we run tests to check whether Ice.MemoryLimitException is
	// raised as expected.
	//
	properties->setProperty("Ice.MessageSizeMax", "100");
	//properties->setProperty("Ice.Trace.Network", "5");
	//properties->setProperty("Ice.Trace.Protocol", "5");
	
	loadConfig(properties);
	setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        Test::MyClassPrx allTests(const Ice::CommunicatorPtr&);
        Test::MyClassPrx myClass = allTests(communicator());

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
