// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestApplication.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AdapterDeactivationTestApplication : public TestApplication
{
public:

    AdapterDeactivationTestApplication()
        : TestApplication("adapter deactivation client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::PropertiesPtr properties = Ice::createProperties();
	loadConfig(properties);
        setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        TestIntfPrx allTests(const CommunicatorPtr&);
        TestIntfPrx obj = allTests(communicator());

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
