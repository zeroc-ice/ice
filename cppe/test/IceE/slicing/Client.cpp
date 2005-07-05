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
        setCommunicator(IceE::initialize(argc, argv));

        TestIntfPrx allTests(const IceE::CommunicatorPtr&);
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
