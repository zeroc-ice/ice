// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
        Ice::InitializationData initData;
	initData.properties = Ice::createProperties();

	//
	// Set a default value for "Hello.Proxy" so that the demo will
	// run without a configuration file.
	//
	initData.properties->setProperty("Hello.Proxy", "hello:tcp -p 10000");

	//
	// Now, load the configuration file if present. Under WinCE we
	// use "config.txt" since it can be edited with pocket word.
	//
	try
	{
	    initData.properties->load("config.txt");
	}
	catch(const Ice::FileException&)
	{
	}

	communicator = Ice::initialize(__argc, __argv, initData);

	HelloPrx hello =
	    HelloPrx::checkedCast(communicator->stringToProxy(initData.properties->getProperty("Hello.Proxy")));
	if(!hello)
	{
	    MessageBox(NULL, L"invalid proxy", L"Minimal Client", MB_ICONEXCLAMATION | MB_OK);
	    return EXIT_FAILURE;
	}

	hello->sayHello();
	MessageBox(NULL, L"Sent \"sayHello()\" message", L"Minimal Client", MB_ICONEXCLAMATION | MB_OK);
    }
    catch(const Ice::Exception& ex)
    {
	TCHAR wtext[1024];
	string err = ex.toString();
	mbstowcs(wtext, err.c_str(), err.size());
	MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

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
	    TCHAR wtext[1024];
	    string err = ex.toString();
	    mbstowcs(wtext, err.c_str(), err.size());
	    MessageBox(NULL, wtext, L"Error", MB_ICONEXCLAMATION | MB_OK);

	    status = EXIT_FAILURE;
	}
    }

    return status;
}
