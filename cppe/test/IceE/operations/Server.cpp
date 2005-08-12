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
	Ice::PropertiesPtr properties = Ice::createProperties();

	properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
	//properties->setProperty("Ice.Trace.Network", "5");
	//properties->setProperty("Ice.Trace.Protocol", "5");

	loadConfig(properties);
	setCommunicator(Ice::initializeWithProperties(argc, argv, properties));
	
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new MyDerivedClassI(adapter, Ice::stringToIdentity("test"));
        adapter->add(object, Ice::stringToIdentity("test"));
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
