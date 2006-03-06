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
	Ice::PropertiesPtr properties = Ice::createProperties();

	//
	// We must set MessageSizeMax to an explicit values, because
	// we run tests to check whether Ice.MemoryLimitException is
	// raised as expected.
	//
	properties->setProperty("Ice.MessageSizeMax", "100");
	properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");
	//properties->setProperty("Ice.Trace.Network", "5");
	//properties->setProperty("Ice.Trace.Protocol", "5");

	loadConfig(properties);
	setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
	Ice::Identity id = Ice::stringToIdentity("test");
        adapter->add(new MyDerivedClassI(adapter, id), id);
	adapter->add(new TestCheckedCastI, Ice::stringToIdentity("context"));
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
