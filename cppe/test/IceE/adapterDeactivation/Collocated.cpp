// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
        : TestApplication("adapter deactivation collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::PropertiesPtr properties = Ice::createProperties();
	properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");

        loadConfig(properties);
        setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        Ice::ObjectPtr object = new TestI;
        adapter->add(object, Ice::stringToIdentity("test"));
        adapter->activate();

        TestIntfPrx allTests(const CommunicatorPtr&);
        allTests(communicator());

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
