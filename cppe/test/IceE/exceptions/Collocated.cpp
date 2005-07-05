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

class ExceptionsTestApplication : public TestApplication
{
public:

    ExceptionsTestApplication() :
        TestApplication("exceptions collocated")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
	IceE::PropertiesPtr properties = IceE::getDefaultProperties(argc, argv);

	properties->setProperty("TestAdapter.Endpoints", "default -p 12345 -t 10000");
        properties->setProperty("IceE.Warn.Dispatch", "0");

        setCommunicator(IceE::initialize(argc, argv));

        IceE::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("TestAdapter");
        IceE::ObjectPtr object = new ThrowerI(adapter);
        adapter->add(object, IceE::stringToIdentity("thrower"));
        adapter->activate();

        ThrowerPrx allTests(const IceE::CommunicatorPtr&);
        allTests(communicator());

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
