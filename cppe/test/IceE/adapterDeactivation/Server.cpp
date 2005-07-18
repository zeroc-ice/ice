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
using namespace Ice;
using namespace Test;

class AdapterDeactivationTestApplication : public TestApplication
{
public:

    AdapterDeactivationTestApplication()
        : TestApplication("adapter deactivation server")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        setCommunicator(Ice::initialize(argc, argv));

        communicator()->getProperties()->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new TestI;
        adapter->add(object, Ice::stringToIdentity("test"));
        adapter->activate();
        adapter->waitForDeactivate();

        return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    AdapterDeactivationTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    AdapterDeactivationTestApplication app;
    return app.main(argc, argv);
}

#endif
