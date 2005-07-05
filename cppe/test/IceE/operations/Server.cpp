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
        TestApplication("operations server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	IceE::PropertiesPtr properties = IceE::getDefaultProperties(argc, argv);

	properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
	properties->setProperty("CheckedCastAdapter.Endpoints", "default -p 12346 -t 10000");
	//properties->setProperty("IceE.Trace.Network", "5");
	//properties->setProperty("IceE.Trace.Protocol", "5");

	setCommunicator(IceE::initialize(argc, argv));
	
        IceE::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        IceE::ObjectPtr object = new MyDerivedClassI(adapter, IceE::stringToIdentity("test"));
        adapter->add(object, IceE::stringToIdentity("test"));
        adapter->activate();

        //
        // Make a separate adapter with a servant locator. We use this to test
        // that ::IceE::Context is correctly passed to checkedCast() operation.
        //
        adapter = communicator()->createObjectAdapter("CheckedCastAdapter");
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
