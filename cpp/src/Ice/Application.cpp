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
    catch(const std::exception& ex)
    {
	cerr << _appName << ": std::exception: " << ex.what() << endl;
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
    memset(&action, 0, sizeof(action));
    action.sa_handler = interruptHandler;
    sigemptyset(&action.sa_mask);
    sigaddset(&action.sa_mask, SIGHUP);
    sigaddset(&action.sa_mask, SIGINT);
    sigaddset(&action.sa_mask, SIGTERM);
    action.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

void
Ice::Application::ignoreInterrupt()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

void
Ice::Application::defaultInterrupt()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &action, 0);
    sigaction(SIGINT, &action, 0);
    sigaction(SIGTERM, &action, 0);
}

void
Ice::Application::holdInterrupt()
{
    sigset_t sigset;
    sigprocmask(0, 0, &sigset);
    sigaddset(&sigset, SIGHUP);
    sigaddset(&sigset, SIGINT);
    sigaddset(&sigset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sigset, 0);
}

void
Ice::Application::releaseInterrupt()
{
    sigset_t sigset;
    sigprocmask(0, 0, &sigset);
    sigdelset(&sigset, SIGHUP);
    sigdelset(&sigset, SIGINT);
    sigdelset(&sigset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sigset, 0);
}

#endif
