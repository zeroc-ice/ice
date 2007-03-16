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
#include <Test.h>

using namespace std;
using namespace Test;

class SlicingTestApplication : public TestApplication
{
public:

    SlicingTestApplication() :
        TestApplication("slicing client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        loadConfig(initData.properties);
	initData.logger = getLogger();
        setCommunicator(Ice::initialize(argc, argv, initData));

        TestIntfPrx allTests(const Ice::CommunicatorPtr&);
        TestIntfPrx Test = allTests(communicator());
        Test->shutdown();

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
