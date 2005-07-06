// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <TestI.h>

using namespace std;

class ExceptionsTestApplication : public TestApplication
{
public:

    ExceptionsTestApplication() :
        TestApplication("exceptions server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	Ice::PropertiesPtr properties = Ice::getDefaultProperties(argc, argv);
        properties->setProperty("IceE.Warn.Dispatch", "0");
        properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");

        setCommunicator(Ice::initialize(argc, argv));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new ThrowerI(adapter);
        adapter->add(object, Ice::stringToIdentity("thrower"));
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
    ExceptionsTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    ExceptionsTestApplication app;
    return app.main(argc, argv);
}

#endif
