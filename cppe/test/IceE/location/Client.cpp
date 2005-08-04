// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#ifdef ICEE_HAS_LOCATOR

#include <TestApplication.h>
#include <Test.h>

using namespace std;

class LocationTestApplication : public TestApplication
{
public:

    LocationTestApplication() :
        TestApplication("location client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
        loadConfig(properties);

	properties->setProperty("Ice.Default.Locator",
				properties->getPropertyWithDefault(
				    "Location.Locator", "locator:default -p 12345"));
        setCommunicator(Ice::initializeWithProperties(argc, argv, properties));

        void allTests(const Ice::CommunicatorPtr&);
	allTests(communicator());

        return EXIT_SUCCESS;
    }

};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    LocationTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    LocationTestApplication app;
    return app.main(argc, argv);
}

#endif

#else

#include <TestCommon.h>

int
main(int argc, char* argv[])
{
#ifdef _WIN32
    tprintf("%d\n", _getpid());
#else
    tprintf("%d\n", getpid());
#endif
    tprintf("Adapter Ready\n");

    return 0;
}

#endif
