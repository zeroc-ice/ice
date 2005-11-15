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
#include <Ice/GC.h>
#include <memory>

using namespace std;
using namespace Ice;
using namespace IceUtil;

//
// _mutex and _condVar are used to synchronize the main thread and
// the CtrlCHandler thread
//
static StaticMutex _mutex = ICE_STATIC_MUTEX_INITIALIZER;
static auto_ptr<Cond> _condVar;

//
// Variables than can change while run() and communicator->destroy() are running!
//
static bool _callbackInProgress = false;
static bool _destroyed = false;
static bool _interrupted = false;
static bool _released = false;
static CtrlCHandlerCallback _previousCallback = 0;

//
// Variables that are immutable during run() and until communicator->destroy() has returned;
// before and after run(), and once communicator->destroy() has returned, we assume that 
// only the main thread and CtrlCHandler threads are running.
//
static const char* _appName = 0;
static CommunicatorPtr _communicator;
static CtrlCHandler* _ctrlCHandler = 0;
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
    CtrlCHandlerCallback callback = 0;
    {
	StaticMutex::Lock lock(_mutex);
	while(!_released)
	{
	    _condVar->wait(lock);
	}
	
	if(_destroyed)
	{
	    //
	    // Being destroyed by main thread
	    //
	    return;
	}
	assert(_ctrlCHandler != 0);
	callback = _ctrlCHandler->getCallback();
    }
    
    if(callback != 0)
    {
	callback(signal);
    }
}

static void destroyOnInterruptCallback(int signal)
{
    {
	StaticMutex::Lock lock(_mutex);
	if(_destroyed)
	{
	    //
	    // Being destroyed by main thread
	    //
	    return;
	}
	if(_nohup && signal == SIGHUP)
	{
	    return;
	}

	assert(!_callbackInProgress);
	_callbackInProgress = true;
	_interrupted = true;
	_destroyed = true;
    }
	
    assert(_communicator != 0);
    
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

    {
	StaticMutex::Lock lock(_mutex);
	_callbackInProgress = false;
    }
    _condVar->signal();
}


static void shutdownOnInterruptCallback(int signal)
{
    {
	StaticMutex::Lock lock(_mutex);
	if(_destroyed)
	{
	    //
	    // Being destroyed by main thread
	    //
	    return;
	}
	if(_nohup && signal == SIGHUP)
	{
	    return;
	}
	assert(!_callbackInProgress);
	_callbackInProgress = true;
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

    {
	StaticMutex::Lock lock(_mutex);
	_callbackInProgress = false;
    }
    _condVar->signal();

}


Ice::Application::Application()
{
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile, const LoggerPtr& logger)
{
    if(_communicator != 0)
    {
	cerr << argv[0] << ": only one instance of the Application class can be used" << endl;
	return EXIT_FAILURE;
    }

    if(_condVar.get() == 0)
    {
	_condVar.reset(new Cond);
    }

    int status;

    try
    {
	//
	// The ctrlCHandler must be created before starting any thread, in particular
	// before initializing the communicator.
	//
	CtrlCHandler ctrCHandler;
	_ctrlCHandler = &ctrCHandler;

	try
	{
	    _interrupted = false;
	    _appName = argv[0];
	  	
	
	    if(configFile)
	    {
		PropertiesPtr properties = createProperties();
		properties->load(configFile);
		_communicator = initializeWithPropertiesAndLogger(argc, argv, properties, logger);
	    }
	    else
	    {
		_communicator = initializeWithLogger(argc, argv, logger);
	    }
	    _destroyed = false;

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
	catch(const IceUtil::Exception& ex)
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

	//
	// Don't want any new interrupt
	// And at this point (post-run), it would not make sense to release a held signal to run shutdown or destroy.
	//
	ignoreInterrupt();

	{
	    StaticMutex::Lock lock(_mutex);
	    while(_callbackInProgress)
	    {
		_condVar->wait(lock);
	    }
	    if(_destroyed)
	    {
		_communicator = 0;
	    }
	    else
	    {
		_destroyed = true;
		//
		// And _communicator != 0, meaning will be destroyed next,
		// _destroyed = true also ensures that any remaining callback won't do anything
		//
	    }
	}

	if(_communicator != 0)
	{  
	    try
	    {
		_communicator->destroy();
	    }
	    catch(const IceUtil::Exception& ex)
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

	//
	// Set _ctrlCHandler to 0 only once communicator->destroy() has completed.
	// 
	_ctrlCHandler = 0;
    }
    catch(const CtrlCHandlerException&)
    {
	cerr << argv[0] << ": only one instance of the Application class can be used" << endl;
	status = EXIT_FAILURE;
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
    //
    // if _ctrlCHandler == 0, it's really a bug in the caller
    //

    if(_ctrlCHandler != 0)
    {
	StaticMutex::Lock lock(_mutex); // we serialize all the interrupt-setting
	if(_ctrlCHandler->getCallback() == holdInterruptCallback)
	{
	    _released = true;
	    _ctrlCHandler->setCallback(destroyOnInterruptCallback);
	    _condVar->signal();
	}
	else
	{
	    _ctrlCHandler->setCallback(destroyOnInterruptCallback);
	}
    }
}

void
Ice::Application::shutdownOnInterrupt()
{
    if(_ctrlCHandler != 0)
    {
	StaticMutex::Lock lock(_mutex); // we serialize all the interrupt-setting
	if(_ctrlCHandler->getCallback() == holdInterruptCallback)
	{
	    _released = true;
	    _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
	    _condVar->signal();
	}
	else
	{
	    _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
	}
    }
}

void
Ice::Application::ignoreInterrupt()
{
    if(_ctrlCHandler != 0)
    {
	StaticMutex::Lock lock(_mutex); // we serialize all the interrupt-setting
	if(_ctrlCHandler->getCallback() == holdInterruptCallback)
	{
	    _released = true;
	    _ctrlCHandler->setCallback(0);
	    _condVar->signal();
	}
	else
	{
	    _ctrlCHandler->setCallback(0);
	}
    }
}

void
Ice::Application::holdInterrupt()
{
    if(_ctrlCHandler != 0)
    {
	StaticMutex::Lock lock(_mutex); // we serialize all the interrupt-setting
	if(_ctrlCHandler->getCallback() != holdInterruptCallback)
	{
	    _previousCallback = _ctrlCHandler->getCallback();
	    _released = false;
	    _ctrlCHandler->setCallback(holdInterruptCallback);
	}
	// else, we were already holding signals
    }
}

void
Ice::Application::releaseInterrupt()
{
    if(_ctrlCHandler != 0)
    {
	StaticMutex::Lock lock(_mutex); // we serialize all the interrupt-setting
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
	    _condVar->signal();
	}
	// Else nothing to release.
    }
}

bool
Ice::Application::interrupted()
{
    StaticMutex::Lock lock(_mutex);
    return _interrupted;
}
