// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Application.h>

using namespace std;
using namespace Ice;

const char* Application::_appName = 0;
CommunicatorPtr Application::_communicator;

bool Application::_interrupted = false;

#ifndef _WIN32
const int Application::signals[] = { SIGHUP, SIGINT, SIGTERM };
sigset_t Application::signalSet;
#endif

Ice::Application::Application()
{
#ifndef _WIN32
    sigemptyset(&signalSet);
    for (unsigned i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
    {
	sigaddset(&signalSet, signals[i]);
    }
#endif
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

    Application::_interrupted = false;
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
    catch(const std::exception& ex)
    {
	cerr << _appName << ": std::exception: " << ex.what() << endl;
	status = EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
	cerr << _appName << ": " << msg << endl;
	status = EXIT_FAILURE;
    }
    catch(const char * msg)
    {
	cerr << _appName << ": " << msg << endl;
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
	catch(const std::exception& ex)
	{
	    cerr << _appName << ": std::exception: " << ex.what() << endl;
	    status = EXIT_FAILURE;
	}
	catch(const std::string& msg)
	{
	    cerr << _appName << ": " << msg << endl;
	    status = EXIT_FAILURE;
	}
	catch(const char * msg)
	{
	    cerr << _appName << ": " << msg << endl;
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
    Application::_interrupted = true;

    //
    // Don't use Application::communicator(), this is not signal-safe.
    //
    assert(Application::_communicator);
    Application::_communicator->signalShutdown();
    return TRUE;
}

enum InterruptDisposition { Shutdown, Ignore, Default };
static InterruptDisposition currentDisposition = Default;

void
Ice::Application::shutdownOnInterrupt()
{
    SetConsoleCtrlHandler(NULL, FALSE);
    SetConsoleCtrlHandler(interruptHandler, TRUE);
    currentDisposition = Shutdown;
}

void
Ice::Application::ignoreInterrupt()
{
    SetConsoleCtrlHandler(interruptHandler, FALSE);
    SetConsoleCtrlHandler(NULL, TRUE);
    currentDisposition = Ignore;
}

void
Ice::Application::defaultInterrupt()
{
    SetConsoleCtrlHandler(interruptHandler, FALSE);
    SetConsoleCtrlHandler(NULL, FALSE);
    currentDisposition = Default;
}

void
Ice::Application::holdInterrupt()
{
    //
    // With Windows, we can't block signals and
    // remember them the way we can with UNIX,
    // so "holding" an interrupt is the same as
    // ignoring it.
    //
    SetConsoleCtrlHandler(interruptHandler, FALSE);
    SetConsoleCtrlHandler(NULL, TRUE);
}

void
Ice::Application::releaseInterrupt()
{
    //
    // Restore current signal disposition.
    //
    switch(currentDisposition)
    {
	case Shutdown:
	{
	    shutdownOnInterrupt();
	    break;
	}
	case Ignore:
	{
	    ignoreInterrupt();
	    break;
	}
	case Default:
	{
	    defaultInterrupt();
	    break;
	}
	default:
	{
	    assert(false);
	    break;
	}
    }
}

#else

void
Ice::interruptHandler(int)
{
    Application::_interrupted = true;

    //
    // Don't use Application::communicator(), this is not signal-safe.
    //
    assert(Application::_communicator);
    Application::_communicator->signalShutdown();
}

void
Ice::Application::shutdownOnInterrupt()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = interruptHandler;
    action.sa_mask = signalSet;
    action.sa_flags = SA_RESTART;
    for (unsigned i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
    {
	sigaction(signals[i], &action, 0);
    }
}

void
Ice::Application::ignoreInterrupt()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    action.sa_mask = signalSet;
    action.sa_flags = SA_RESTART;
    for (unsigned i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
    {
	sigaction(signals[i], &action, 0);
    }
}

void
Ice::Application::defaultInterrupt()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_DFL;
    action.sa_mask = signalSet;
    action.sa_flags = SA_RESTART;
    for (unsigned i = 0; i < sizeof(signals) / sizeof(*signals); ++i)
    {
	sigaction(signals[i], &action, 0);
    }
}

void
Ice::Application::holdInterrupt()
{
    sigprocmask(SIG_BLOCK, &signalSet, 0);
}

void
Ice::Application::releaseInterrupt()
{
    sigprocmask(SIG_UNBLOCK, &signalSet, 0);
}

#endif

bool
Ice::Application::interrupted()
{
    return Application::_interrupted;
}
