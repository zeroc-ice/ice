// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	Ice::PropertiesPtr properties = Ice::getDefaultProperties(argc, argv);
	
	//
	// We must set MessageSizeMax to an explicit values, because
	// we run tests to check whether IceE.MemoryLimitException is
	// raised as expected.
	//
	properties->setProperty("IceE.MessageSizeMax", "100");
	properties->setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
        properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
	//properties->setProperty("IceE.Trace.Protocol", "10");
	//properties->setProperty("IceE.Trace.Network", "10");
	
	setCommunicator(Ice::initialize(argc, argv));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new MyDerivedClassI(adapter, Ice::stringToIdentity("test"));
        adapter->add(object, Ice::stringToIdentity("test"));
        adapter->activate();

        //
        // Make a separate adapter with a servant locator. We use this to test
        // that ::Ice::Context is correctly passed to checkedCast() operation.
        //
        adapter = communicator()->createObjectAdapter("CheckedCastAdapter");
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
