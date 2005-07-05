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
using namespace Test;

class FacetsTestApplication : public TestApplication
{
public:

    FacetsTestApplication() :
        TestApplication("facets collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	IceE::PropertiesPtr properties = IceE::getDefaultProperties(argc, argv);
	properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");

        setCommunicator(IceE::initialize(argc, argv));

        IceE::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        IceE::ObjectPtr d = new DI;
        adapter->add(d, IceE::stringToIdentity("d"));
        adapter->addFacet(d, IceE::stringToIdentity("d"), "facetABCD");
        IceE::ObjectPtr f = new FI;
        adapter->addFacet(f, IceE::stringToIdentity("d"), "facetEF");
        IceE::ObjectPtr h = new HI(communicator());
        adapter->addFacet(h, IceE::stringToIdentity("d"), "facetGH");
        adapter->activate();

        GPrx allTests(const IceE::CommunicatorPtr&);
        allTests(communicator());

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
