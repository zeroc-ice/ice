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

class FacetsTestApplication : public TestApplication
{
public:

    FacetsTestApplication() :
        TestApplication("facets server")
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
        Ice::ObjectPtr d = new DI;
        adapter->add(d, communicator()->stringToIdentity("d"));
        adapter->addFacet(d, communicator()->stringToIdentity("d"), "facetABCD");
        Ice::ObjectPtr f = new FI;
        adapter->addFacet(f, communicator()->stringToIdentity("d"), "facetEF");
        Ice::ObjectPtr h = new HI(communicator());
        adapter->addFacet(h, communicator()->stringToIdentity("d"), "facetGH");

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
    FacetsTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    FacetsTestApplication app;
    return app.main(argc, argv);
}

#endif
