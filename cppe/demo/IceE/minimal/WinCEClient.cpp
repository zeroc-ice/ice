// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    extern int    __argc;
    extern char **__argv; 

    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(__argc, __argv);
	HelloPrx hello = HelloPrx::checkedCast(communicator->stringToProxy("hello:tcp -p 10000"));
	if(!hello)
	{
	    MessageBox(NULL, L"invalid proxy", L"Minimal Client", MB_ICONEXCLAMATION | MB_OK);
	    return EXIT_FAILURE;
	}
	hello->sayHello();
    }
    catch(const Ice::Exception& ex)
    {
	char buf[1024];
        sprintf(buf, "%s\n", ex.toString().c_str());
	TCHAR wtext[1024];
	mbstowcs(wtext, buf, strlen(buf));
	MessageBox(NULL, wtext, L"Minimal Client", MB_ICONEXCLAMATION | MB_OK);
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    fprintf(stderr, "%s\n", ex.toString().c_str());
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
