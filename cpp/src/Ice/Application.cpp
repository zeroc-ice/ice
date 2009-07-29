// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/LoggerI.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Cond.h>
#include <IceUtil/ArgVector.h>
#include <Ice/GC.h>
#include <memory>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceUtilInternal;

//
// _mutex and _condVar are used to synchronize the main thread and
// the CtrlCHandler thread
//

namespace
{

Mutex* mutex = 0;

class Init
{
public:

    Init()
    {
        mutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete mutex;
        mutex = 0;
    }
};

Init init;

auto_ptr<Cond> _condVar;

//
// Variables than can change while run() and communicator->destroy() are running!
//
bool _callbackInProgress = false;
bool _destroyed = false;
bool _interrupted = false;
bool _released = false;
CtrlCHandlerCallback _previousCallback = 0;

//
// Variables that are immutable during run() and until communicator->destroy() has returned;
// before and after run(), and once communicator->destroy() has returned, we assume that 
// only the main thread and CtrlCHandler threads are running.
//
string _appName;
Application* _application;
CommunicatorPtr _communicator;
CtrlCHandler* _ctrlCHandler = 0;
bool _nohup = false;
SignalPolicy _signalPolicy = HandleSignals;

}

#ifdef _WIN32
const DWORD SIGHUP = CTRL_LOGOFF_EVENT;
#else
#   include <csignal>
#endif

//
// Compaq C++ defines signal() as a macro, causing problems with the _condVar->signal()
// statement, which the compiler for some reason replaces by the macro.
//
#if defined (__digital__) && defined (__unix__)
#   undef signal
#endif

//
// CtrlCHandler callbacks.
//

static void
holdInterruptCallback(int signal)
{
    CtrlCHandlerCallback callback = 0;
    {
        IceUtil::Mutex::Lock lock(*mutex);
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

static void
destroyOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*mutex);
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
        
    try
    {
        assert(_communicator != 0);
        _communicator->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while destroying in response to signal " << signal << "): " << ex.what();
#ifdef __GNUC__
        out << "\n" << ex.ice_stackTrace();
#endif
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while destroying in response to signal " << signal << "): " << ex.what();
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while destroying in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while destroying in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << _appName << " (while destroying in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*mutex);
        _callbackInProgress = false;
    }
    _condVar->signal();
}

static void
shutdownOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*mutex);
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
        assert(_communicator != 0);
        _communicator->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while shutting down in response to signal " << signal << "): std::exception: "
            << ex.what();
#ifdef __GNUC__
        out << "\n" << ex.ice_stackTrace();
#endif
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while shutting down in response to signal " << signal << "): std::exception: "
            << ex.what();
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << _appName << " (while shutting down in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*mutex);
        _callbackInProgress = false;
    }
    _condVar->signal();
}

static void
callbackOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*mutex);
        if(_destroyed)
        {
            //
            // Being destroyed by main thread
            //
            return;
        }
        // For SIGHUP the user callback is always called. It can
        // decide what to do.
        assert(!_callbackInProgress);
        _callbackInProgress = true;
        _interrupted = true;
    }

    try
    {
        assert(_application != 0);
        _application->interruptCallback(signal);
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while interrupting in response to signal " << signal << "): std::exception: "
             << ex.what();
#ifdef __GNUC__
        out << "\n" << ex.ice_stackTrace();
#endif
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << _appName << " (while interrupting in response to signal " << signal << "): std::exception: "
             << ex.what();
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while interrupting in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << _appName << " (while interrupting in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << _appName << " (while interrupting in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*mutex);
        _callbackInProgress = false;
    }
    _condVar->signal();
}

Ice::Application::Application(SignalPolicy signalPolicy)
{
    _signalPolicy = signalPolicy;
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, char* argv[])
{
    return mainInternal(argc, argv, InitializationData(), 0);
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile)
{
    return main(argc, argv, configFile, 0);
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile, const Ice::PropertiesPtr& overrideProps)
{
    //
    // We don't call the main below to avoid a deprecated warning
    //

    InitializationData initData;
    if(configFile)
    {
        try
        {
            initData.properties = createProperties();
            initData.properties->load(configFile);
        }
        catch(const Ice::Exception& ex)
        {
            Error out(getProcessLogger());
	    if(argv[0])
	    {
		out << argv[0] << ": ";
	    }
	    out << ex.what();
#ifdef __GNUC__
            out << "\n" << ex.ice_stackTrace();
#endif
            return EXIT_FAILURE;
        }
        catch(const std::exception& ex)
        {
            Error out(getProcessLogger());
	    if(argv[0])
	    {
		out << argv[0] << ": ";
	    }
	    out << ex.what();
            return EXIT_FAILURE;
        }
        catch(...)
        {
            Error out(getProcessLogger());
	    if(argv[0])
	    {
		out << argv[0] << ": ";
	    }
            out << "unknown exception";
            return EXIT_FAILURE;
        }
    }
    return mainInternal(argc, argv, initData, overrideProps);
}

#ifdef _WIN32

int
Ice::Application::main(int argc, wchar_t* argv[])
{
    return main(argsToStringSeq(argc, argv));
}

int
Ice::Application::main(int argc, wchar_t* argv[], const char* config)
{
    return main(argsToStringSeq(argc, argv), config);
}

int
Ice::Application::main(int argc, wchar_t* argv[], const char* config, const Ice::PropertiesPtr& properties)
{
    return main(argsToStringSeq(argc, argv), config, properties);
}

int
Ice::Application::main(int argc, wchar_t* argv[], const Ice::InitializationData& initData)
{
    return main(argsToStringSeq(argc, argv, initData.stringConverter), initData);
}

#endif

int
Ice::Application::main(int argc, char* argv[], const InitializationData& initData)
{
    return mainInternal(argc, argv, initData, 0);
}

int
Ice::Application::main(const StringSeq& args)
{
    ArgVector av(args);
    return main(av.argc, av.argv);
}

int
Ice::Application::main(const StringSeq& args, const char* configFile)
{
    ArgVector av(args);
    return main(av.argc, av.argv, configFile);
}

int
Ice::Application::main(const StringSeq& args, const char* configFile, const Ice::PropertiesPtr& overrideProps)
{
    ArgVector av(args);
    return main(av.argc, av.argv, configFile, overrideProps);
}

int
Ice::Application::main(const StringSeq& args, const InitializationData& initData)
{
    ArgVector av(args);
    return main(av.argc, av.argv, initData);
}

void
Ice::Application::interruptCallback(int)
{
}

const char*
Ice::Application::appName()
{
    return _appName.c_str();
}

CommunicatorPtr
Ice::Application::communicator()
{
    return _communicator;
}

void
Ice::Application::destroyOnInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                _condVar->signal();
            }
            _ctrlCHandler->setCallback(destroyOnInterruptCallback);
        }
    }
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::shutdownOnInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                _condVar->signal();
            }
            _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
        }
    }
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts."; 
    }
}

void
Ice::Application::ignoreInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                _condVar->signal();
            }
            _ctrlCHandler->setCallback(0);
        }
    }
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::callbackOnInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                _condVar->signal();
            }
            _ctrlCHandler->setCallback(callbackOnInterruptCallback);
        }
    }
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::holdInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() != holdInterruptCallback)
            {
                _previousCallback = _ctrlCHandler->getCallback();
                _released = false;
                _ctrlCHandler->setCallback(holdInterruptCallback);
            }
            // else, we were already holding signals
        }
    }
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::releaseInterrupt()
{
    if(_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*mutex); // we serialize all the interrupt-setting
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
    else
    {
        Error out(getProcessLogger());
        out << _appName << ": warning: interrupt method called on Application configured to not handle interrupts.";
    }
}

bool
Ice::Application::interrupted()
{
    IceUtil::Mutex::Lock lock(*mutex);
    return _interrupted;
}

int
Ice::Application::mainInternal(int argc, char* argv[], const InitializationData& initData,
                               const Ice::PropertiesPtr& overrideProps)
{
    if(_communicator != 0)
    {
        Error out(getProcessLogger());
	if(argv[0])
	{
	    out << argv[0] << ": ";
	}
        out << "only one instance of the Application class can be used";
        return EXIT_FAILURE;
    }
    int status;

    if(_signalPolicy == HandleSignals)
    {
        try
        {
            //
            // The ctrlCHandler must be created before starting any thread, in particular
            // before initializing the communicator.
            //
            CtrlCHandler ctrCHandler;
            _ctrlCHandler = &ctrCHandler;

            status = executeRun(argc, argv, initData, overrideProps);

            //
            // Set _ctrlCHandler to 0 only once communicator->destroy() has completed.
            // 
            _ctrlCHandler = 0;
        }
        catch(const CtrlCHandlerException&)
        {
            Error out(getProcessLogger());
	    if(argv[0])
	    {
		out << argv[0] << ": ";
	    }
            out << "only one instance of the CtrlCHandler class can be used";
            status = EXIT_FAILURE;
        }
    }
    else
    {
        status = executeRun(argc, argv, initData, overrideProps);
    }
   
    return status;
}

int
Ice::Application::executeRun(int argc, char* argv[], const InitializationData& initializationData, 
                             const Ice::PropertiesPtr& overrideProps)
{
    int status;

    try
    {
        if(_condVar.get() == 0)
        {
            _condVar.reset(new Cond);
        }

        _interrupted = false;
	_appName = argv[0] ? argv[0] : "";

        //
        // We parse the properties here to extract Ice.ProgramName.
        // 
        InitializationData initData = initializationData;
        initData.properties = createProperties(argc, argv, initData.properties, initData.stringConverter);
        if(overrideProps)
        {
            Ice::PropertyDict props = overrideProps->getPropertiesForPrefix("");
            for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
            {
                initData.properties->setProperty(p->first, p->second);
            }
        }

        //
        // If the process logger is the default logger, we replace it with a
        // a logger which is using the program name for the prefix.
        //
        if(LoggerIPtr::dynamicCast(getProcessLogger()))
        {
            setProcessLogger(new LoggerI(initData.properties->getProperty("Ice.ProgramName"), ""));
        }

        _application = this;
        _communicator = initialize(argc, argv, initData);
        _destroyed = false;

        //
        // Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
        //
        _nohup = (_communicator->getProperties()->getPropertyAsInt("Ice.Nohup") > 0);
    
        //
        // The default is to destroy when a signal is received.
        //
        if(_signalPolicy == HandleSignals)
        {
            destroyOnInterrupt();
        }

        status = run(argc, argv);
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << ": " << ex.what();
#ifdef __GNUC__
        out << "\n" << ex.ice_stackTrace();
#endif
        status = EXIT_FAILURE;
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << ": " << ex.what();
        status = EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << _appName << ": " << msg;
        status = EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << _appName << ": " << msg;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << _appName << ": unknown exception";
        status = EXIT_FAILURE;
    }

    //
    // Don't want any new interrupt and at this point (post-run),
    // it would not make sense to release a held signal to run
    // shutdown or destroy.
    //
    if(_signalPolicy == HandleSignals)
    {
        ignoreInterrupt();
    }

    {
        IceUtil::Mutex::Lock lock(*mutex);
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
            // And _communicator != 0, meaning will be destroyed
            // next, _destroyed = true also ensures that any
            // remaining callback won't do anything
            //
        }
        _application = 0;
    }

    if(_communicator != 0)
    {  
        try
        {
            _communicator->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            Error out(getProcessLogger());
            out << _appName << ": " << ex.what();
#ifdef __GNUC__
            out << "\n" << ex.ice_stackTrace();
#endif
            status = EXIT_FAILURE;
        }
        catch(const std::exception& ex)
        {
            Error out(getProcessLogger());
            out << _appName << ": " << ex.what();
            status = EXIT_FAILURE;
        }
        catch(...)
        {
            Error out(getProcessLogger());
            out << _appName << ": unknown exception";
            status = EXIT_FAILURE;
        }
        _communicator = 0;
    }

    return status;
}
