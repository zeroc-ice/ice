// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#ifndef _WIN32
#   include <csignal>
#endif

using namespace std;
using namespace Ice;

const char* Application::_appName = 0;
CommunicatorPtr Application::_communicator;

Ice::Application::Application()
{
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile)
{
    if(_communicator)
    {
	cerr << argv[0] << ": only one instance of the Application class can be used" << endl;
	return EXIT_FAILURE;
    }

    _appName = argv[0];

    int status;

    try
    {
	if(configFile)
	{
	    PropertiesPtr properties = createProperties(argc, argv);
            properties->load(configFile);
	    _communicator = initializeWithProperties(argc, argv, properties);
	}
	else
	{
	    _communicator = initialize(argc, argv);
	}
	status = run(argc, argv);
    }
    catch(const Exception& ex)
    {
	cerr << _appName << ": " << ex << endl;
	status = EXIT_FAILURE;
    }
    catch(...)
    {
	cerr << _appName << ": unknown exception" << endl;
	status = EXIT_FAILURE;
    }

    if(_communicator)
    {
	try
	{
	    _communicator->destroy();
	}
	catch(const Exception& ex)
	{
	    cerr << _appName << ": " << ex << endl;
	    status = EXIT_FAILURE;
	}
	catch(...)
	{
	    cerr << _appName << ": unknown exception" << endl;
	    status = EXIT_FAILURE;
	}
	_communicator = 0;
    }

    return status;
}

const char*
Ice::Application::appName()
{
    return _appName;
}

CommunicatorPtr
Ice::Application::communicator()
{
    return _communicator;
}

#ifdef _WIN32

BOOL WINAPI
Ice::interruptHandler(DWORD)
{
    //
    // Don't use Application::communicator(), this is not signal-safe.
    //
    assert(Application::_communicator);
    Application::_communicator->shutdown();
    return TRUE;
}

void
Ice::Application::shutdownOnInterrupt()
{
    SetConsoleCtrlHandler(NULL, FALSE);
    SetConsoleCtrlHandler(interruptHandler, TRUE);
}

void
Ice::Application::ignoreInterrupt()
{
    SetConsoleCtrlHandler(interruptHandler, FALSE);
    SetConsoleCtrlHandler(NULL, TRUE);
}

void
Ice::Application::defaultInterrupt()
{
    SetConsoleCtrlHandler(interruptHandler, FALSE);
    SetConsoleCtrlHandler(NULL, FALSE);
}

#else

void
Ice::interruptHandler(int)
{
    //
    // Don't use Application::communicator(), this is not signal-safe.
    //
    assert(Application::_communicator);
    Application::_communicator->shutdown();
}

void
Ice::Application::shutdownOnInterrupt()
{
    struct sigaction action;
    action.sa_handler = interruptHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGHUP);
    sigaddset(&action.sa_mask, SIGINT);
    sigaddset(&action.sa_mask, SIGTERM);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

void
Ice::Application::ignoreInterrupt()
{
    struct sigaction action;
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

void
Ice::Application::defaultInterrupt()
{
    struct sigaction action;
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

#endif
