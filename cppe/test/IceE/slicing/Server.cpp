// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

class SlicingTestApplication : public TestApplication
{
public:

    SlicingTestApplication() :
        TestApplication("slicing server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->setProperty("Ice.Warn.Dispatch", "0");
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 2000");

        loadConfig(initData.properties);
	initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new TestI(adapter);
        adapter->add(object, communicator()->stringToIdentity("Test"));
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
    SlicingTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    SlicingTestApplication app;
    return app.main(argc, argv);
}

#endif
