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

class InheritanceTestApplication : public TestApplication
{
public:

    InheritanceTestApplication() :
        TestApplication("inheritance server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
    	Ice::InitializationData initData;
	initData.properties = Ice::createProperties();
        initData.properties->setProperty("TestAdapter.Endpoints", "default -p 12010 -t 10000");

        loadConfig(initData.properties);
	initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new InitialI(adapter);
        adapter->add(object, communicator()->stringToIdentity("initial"));
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
    InheritanceTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    InheritanceTestApplication app;
    return app.main(argc, argv);
}

#endif
