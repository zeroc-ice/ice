// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>

#include <stdlib.h>
#include <TestApplication.h>

#include <TestSuite.h>

using namespace std;

class ThreadTestApplication : public TestApplication
{
public:

    ThreadTestApplication() :
        TestApplication("thread test")
    {
    }

    virtual int run(int argc, char* argv[])
    {
	try
	{
	    initializeTestSuite();

	    for(list<TestBasePtr>::const_iterator p = allTests.begin(); p != allTests.end(); ++p)
	    {
		(*p)->start();
	    }
	}
	catch(const TestFailed& e)
	{
	    tprintf("test %s failed\n", e.name.c_str());
	    return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
    }
};

#ifdef _WIN32_WCE

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    ThreadTestApplication app;
    return app.main(hInstance);
}

#else

int
main(int argc, char** argv)
{
    ThreadTestApplication app;
    return app.main(argc, argv);
}

#endif
