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

class InheritanceTestApplication : public TestApplication
{
public:

    InheritanceTestApplication() :
        TestApplication("inheritance client")
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        setCommunicator(Ice::initialize(argc, argv));

        InitialPrx allTests(const Ice::CommunicatorPtr&);
        InitialPrx initial = allTests(communicator());
        initial->shutdown();

        return EXIT_SUCCESS;
    }

};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    InheritanceTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    InheritanceTestApplication app;
    return app.main(argc, argv);
}

#endif
