// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IceUtil/StaticMutex.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Cond.h>
#include <IceUtil/GC.h>
#include <memory>

using namespace std;
using namespace Ice;
using namespace IceUtil;

static const char* _appName = 0;
static CommunicatorPtr _communicator;

static bool _interrupted = false;
static StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;

static bool _released = false;
static auto_ptr<Cond> _condVar;
static auto_ptr<IceUtil::CtrlCHandler> _ctrlCHandler;
static IceUtil::CtrlCHandlerCallback _previousCallback = 0;
static bool _nohup = false;

#ifdef _WIN32
const DWORD SIGHUP = CTRL_LOGOFF_EVENT;
#else
#   include <csignal>
#endif

//
// CtrlCHandler callbacks.
//

static void holdInterruptCallback(int signal)
{
    {
	StaticMutex::Lock lock(_mutex);
	while(!_released)
	{
	    _condVar->wait(lock);
	}
    }

    //
    // Use the current callback to process this (old) signal.
    //
    CtrlCHandlerCallback callback = _ctrlCHandler->getCallback();
    if(callback != 0)
    {
	callback(signal);
    }
}

static void destroyOnInterruptCallback(int signal)
{
    if(_nohup && signal == SIGHUP)
    {
	return;
    }
	
    {
	StaticMutex::Lock lock(_mutex);
	_interrupted = true;
    }

    try
    {
	_communicator->destroy();
    }
    catch(const IceUtil::Exception& ex)
    {
	cerr << _appName << " (while destroying in response to signal " << signal 
	     << "): " << ex << endl;
    }
    catch(const std::exception& ex)
    {
	cerr << _appName << " (while destroying in response to signal " << signal 
	     << "): std::exception: " << ex.what() << endl;
    }
    catch(const std::string& msg)
    {
	cerr << _appName << " (while destroying in response to signal " << signal
	     << "): " << msg << endl;
    }
    catch(const char * msg)
    {
	cerr << _appName << " (while destroying in response to signal " << signal
	     << "): " << msg << endl;
    }
    catch(...)
    {
	cerr << _appName << " (while destroying in response to signal " << signal 
	     << "): unknown exception" << endl;
    }
}


static void shutdownOnInterruptCallback(int signal)
{
    if(_nohup && signal == SIGHUP)
    {
	return;
    }
	
    {
	StaticMutex::Lock lock(_mutex);
	_interrupted = true;
    }

    try
    {
	_communicator->shutdown();
    }
    catch(const IceUtil::Exception& ex)
    {
	cerr << _appName << " (while shutting down in response to signal " << signal 
	     << "): " << ex << endl;
    }
    catch(const std::exception& ex)
    {
	cerr << _appName << " (while shutting down in response to signal " << signal 
	     << "): std::exception: " << ex.what() << endl;
    }
    catch(const std::string& msg)
    {
	cerr << _appName << " (while shutting down in response to signal " << signal
	     << "): " << msg << endl;
    }
    catch(const char * msg)
    {
	cerr << _appName << " (while shutting down in response to signal " << signal
	     << "): " << msg << endl;
    }
    catch(...)
    {
	cerr << _appName << " (while shutting down in response to signal " << signal 
	     << "): unknown exception" << endl;
    }
}


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

    _interrupted = false;
    _appName = argv[0];
    if(_condVar.get() == 0)
    {
	_condVar.reset(new Cond);
    }

    int status;

    try
    {
	//
	// Ignore signals for a little while.
	//
	//
	// Note that the communicator must be created BEFORE the communicator,
	// since the communicator initialization may create threads.
	//
	_ctrlCHandler.reset(new IceUtil::CtrlCHandler);

	if(configFile)
	{
	    PropertiesPtr properties = createProperties();
            properties->load(configFile);
	    _communicator = initializeWithProperties(argc, argv, properties);
	}
	else
	{
	    _communicator = initialize(argc, argv);
	}

	//
	// Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
	//
	_nohup = (_communicator->getProperties()->getPropertyAsInt("Ice.Nohup") > 0);
	
	//
	// The default is to destroy when a signal is received.
	//
	destroyOnInterrupt();

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
    catch(const char* msg)
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
	//
	// We don't want to handle signals anymore.
	//
	ignoreInterrupt(); // Will release any signal still held.
	_ctrlCHandler.reset();
       
	try
	{
	    _communicator->destroy();
	}
	catch(const Exception& ex)
	{
	    cerr << _appName << ": " << ex << endl;
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

void
Ice::Application::destroyOnInterrupt()
{
    assert(_ctrlCHandler.get() != 0);
    
    StaticMutex::Lock lock(_mutex);
    if(_ctrlCHandler->getCallback() == holdInterruptCallback)
    {
	_released = true;
	_ctrlCHandler->setCallback(destroyOnInterruptCallback);
	lock.release();
	_condVar->signal();
    }
    else
    {
	_ctrlCHandler->setCallback(destroyOnInterruptCallback);
    }
}

void
Ice::Application::shutdownOnInterrupt()
{
    assert(_ctrlCHandler.get() != 0);
    
    StaticMutex::Lock lock(_mutex);
    if(_ctrlCHandler->getCallback() == holdInterruptCallback)
    {
	_released = true;
	_ctrlCHandler->setCallback(shutdownOnInterruptCallback);
	lock.release();
	_condVar->signal();
    }
    else
    {
	_ctrlCHandler->setCallback(shutdownOnInterruptCallback);
    }
}

void
Ice::Application::ignoreInterrupt()
{
    assert(_ctrlCHandler.get() != 0);

    StaticMutex::Lock lock(_mutex);
    if(_ctrlCHandler->getCallback() == holdInterruptCallback)
    {
	_released = true;
	_ctrlCHandler->setCallback(0);
	lock.release();
	_condVar->signal();
    }
    else
    {
	_ctrlCHandler->setCallback(0);
    }
}

void
Ice::Application::holdInterrupt()
{
    assert(_ctrlCHandler.get() != 0);
  
    StaticMutex::Lock lock(_mutex);
    if(_ctrlCHandler->getCallback() != holdInterruptCallback)
    {
	_previousCallback = _ctrlCHandler->getCallback();
	_released = false;
	_ctrlCHandler->setCallback(holdInterruptCallback);
    }
    // else, we were already holding signals
}

void
Ice::Application::releaseInterrupt()
{
    assert(_ctrlCHandler.get() != 0);

    StaticMutex::Lock lock(_mutex);
    if(_ctrlCHandler->getCallback() == holdInterruptCallback)
    {
	//
	// Note that it's very possible no signal is held;
	// in this case the callback is just replaced and
	// setting _released to true and signalling _condVar
	// do no harm.
	//

	_released = true;
	_ctrlCHandler->setCallback(_previousCallback);
	lock.release();
	_condVar->signal();
    }
    // Else nothing to release.
}

bool
Ice::Application::interrupted()
{
    StaticMutex::Lock lock(_mutex);
    return _interrupted;
}
