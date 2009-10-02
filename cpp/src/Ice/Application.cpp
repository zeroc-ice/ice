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
// static initializations.
//
Mutex* IceInternal::mutex = 0;

bool IceInternal::_callbackInProgress = false;
bool IceInternal::_destroyed = false;
bool IceInternal::_interrupted = false;

string IceInternal::_appName;
CommunicatorPtr IceInternal::_communicator;
SignalPolicy IceInternal::_signalPolicy = HandleSignals;
auto_ptr<Cond> IceInternal::_condVar;
Ice::Application* IceInternal::_application;
//
// _mutex and _condVar are used to synchronize the main thread and
// the CtrlCHandler thread
//

namespace
{

class Init
{
public:

    Init()
    {
        IceInternal::mutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete IceInternal::mutex;
        IceInternal::mutex = 0;
    }
};

Init init;



//
// Variables than can change while run() and communicator->destroy() are running!
//
bool _released = false;
CtrlCHandlerCallback _previousCallback = 0;

//
// Variables that are immutable during run() and until communicator->destroy() has returned;
// before and after run(), and once communicator->destroy() has returned, we assume that 
// only the main thread and CtrlCHandler threads are running.
//
CtrlCHandler* _ctrlCHandler = 0;
bool _nohup = false;

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
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        while(!_released)
        {
            IceInternal::_condVar->wait(lock);
        }
        
        if(IceInternal::_destroyed)
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
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        if(IceInternal::_destroyed)
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

        assert(!IceInternal::_callbackInProgress);
        IceInternal::_callbackInProgress = true;
        IceInternal::_interrupted = true;
        IceInternal::_destroyed = true;
    }
        
    try
    {
        assert(IceInternal::_communicator != 0);
        IceInternal::_communicator->destroy();
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << "(while destroying in response to signal " << signal << "): " << ex;
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << "(while destroying in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << "(while destroying in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << "(while destroying in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        IceInternal::_callbackInProgress = false;
    }
    IceInternal::_condVar->signal();
}

static void
shutdownOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        if(IceInternal::_destroyed)
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

        assert(!IceInternal::_callbackInProgress);
        IceInternal::_callbackInProgress = true;
        IceInternal::_interrupted = true;
    }

    try
    {
        assert(IceInternal::_communicator != 0);
        IceInternal::_communicator->shutdown();
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << "(while shutting down in response to signal " << signal << "): std::exception: " << ex;
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << "(while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << "(while shutting down in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << "(while shutting down in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        IceInternal::_callbackInProgress = false;
    }
    IceInternal::_condVar->signal();
}

static void
callbackOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        if(IceInternal::_destroyed)
        {
            //
            // Being destroyed by main thread
            //
            return;
        }
        // For SIGHUP the user callback is always called. It can
        // decide what to do.
        assert(!IceInternal::_callbackInProgress);
        IceInternal::_callbackInProgress = true;
        IceInternal::_interrupted = true;
    }

    try
    {
        assert(IceInternal::_application != 0);
        IceInternal::_application->interruptCallback(signal);
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << "(while interrupting in response to signal " << signal << "): std::exception: " << ex;
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << "(while interrupting in response to signal " << signal << "): " << msg;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << "(while interrupting in response to signal " << signal << "): " << msg;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << "(while interrupting in response to signal " << signal << "): unknown exception";
    }

    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        IceInternal::_callbackInProgress = false;
    }
    IceInternal::_condVar->signal();
}

Ice::Application::Application(SignalPolicy signalPolicy)
{
    IceInternal::_signalPolicy = signalPolicy;
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, char* argv[])
{
    return main(argc, argv, InitializationData());
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile)
{
    //
    // We don't call the main below to avoid a deprecated warning
    //

    if(argc > 0 && argv[0] && LoggerIPtr::dynamicCast(getProcessLogger()))
    {
        setProcessLogger(new LoggerI(argv[0], ""));
    }

    InitializationData initData;
    if(configFile)
    {
        try
        {
            initData.properties = createProperties();
            initData.properties->load(configFile);
        }
        catch(const std::exception& ex)
        {
            Error out(getProcessLogger());
	    out << ex;
            return EXIT_FAILURE;
        }
        catch(...)
        {
            Error out(getProcessLogger());
            out << "unknown exception";
            return EXIT_FAILURE;
        }
    }
    return main(argc, argv, initData);
}

int
Ice::Application::main(int argc, char* argv[], const InitializationData& initData)
{
    if(argc > 0 && argv[0] && LoggerIPtr::dynamicCast(getProcessLogger()))
    {
        setProcessLogger(new LoggerI(argv[0], ""));
    }

    if(IceInternal::_communicator != 0)
    {
        Error out(getProcessLogger());
        out << "only one instance of the Application class can be used";
        return EXIT_FAILURE;
    }
    int status;

    if(IceInternal::_signalPolicy == HandleSignals)
    {
        try
        {
            //
            // The ctrlCHandler must be created before starting any thread, in particular
            // before initializing the communicator.
            //
            CtrlCHandler ctrCHandler;
            _ctrlCHandler = &ctrCHandler;

            status = doMain(argc, argv, initData);

            //
            // Set _ctrlCHandler to 0 only once communicator->destroy() has completed.
            // 
            _ctrlCHandler = 0;
        }
        catch(const CtrlCHandlerException&)
        {
            Error out(getProcessLogger());
            out << "only one instance of the CtrlCHandler class can be used";
            status = EXIT_FAILURE;
        }
    }
    else
    {
        status = doMain(argc, argv, initData);
    }
   
    return status;
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
    return IceInternal::_appName.c_str();
}

CommunicatorPtr
Ice::Application::communicator()
{
    return IceInternal::_communicator;
}

void
Ice::Application::destroyOnInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::_condVar->signal();
            }
            _ctrlCHandler->setCallback(destroyOnInterruptCallback);
        }
    }
    else
    {
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::shutdownOnInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::_condVar->signal();
            }
            _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
        }
    }
    else
    {
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts."; 
    }
}

void
Ice::Application::ignoreInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::_condVar->signal();
            }
            _ctrlCHandler->setCallback(0);
        }
    }
    else
    {
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::callbackOnInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::_condVar->signal();
            }
            _ctrlCHandler->setCallback(callbackOnInterruptCallback);
        }
    }
    else
    {
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::holdInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
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
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts.";
    }
}

void
Ice::Application::releaseInterrupt()
{
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::mutex); // we serialize all the interrupt-setting
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
                IceInternal::_condVar->signal();
            }
            // Else nothing to release.
        }
    }
    else
    {
        Warning out(getProcessLogger());
        out << "interrupt method called on Application configured to not handle interrupts.";
    }
}

bool
Ice::Application::interrupted()
{
    IceUtil::Mutex::Lock lock(*IceInternal::mutex);
    return IceInternal::_interrupted;
}

int
Ice::Application::doMain(int argc, char* argv[], const InitializationData& initializationData)
{
    int status;

    try
    {
        if(IceInternal::_condVar.get() == 0)
        {
            IceInternal::_condVar.reset(new Cond);
        }

        IceInternal::_interrupted = false;
        IceInternal::_appName = argv[0] ? argv[0] : "";

        //
        // We parse the properties here to extract Ice.ProgramName.
        // 
        InitializationData initData = initializationData;
        initData.properties = createProperties(argc, argv, initData.properties, initData.stringConverter);

        //
        // If the process logger is the default logger, we now replace it with a
        // a logger which is using the program name for the prefix.
        //
        if(initData.properties->getProperty("Ice.ProgramName") != "" && LoggerIPtr::dynamicCast(getProcessLogger()))
        {
            setProcessLogger(new LoggerI(initData.properties->getProperty("Ice.ProgramName"), ""));
        }

        IceInternal::_application = this;
        IceInternal::_communicator = initialize(argc, argv, initData);
        IceInternal::_destroyed = false;

        //
        // Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
        //
        _nohup = (IceInternal::_communicator->getProperties()->getPropertyAsInt("Ice.Nohup") > 0);
    
        //
        // The default is to destroy when a signal is received.
        //
        if(IceInternal::_signalPolicy == HandleSignals)
        {
            destroyOnInterrupt();
        }

        status = run(argc, argv);
    }
    catch(const std::exception& ex)
    {
        Error out(getProcessLogger());
        out << ex;
        status = EXIT_FAILURE;
    }
    catch(const std::string& msg)
    {
        Error out(getProcessLogger());
        out << msg;
        status = EXIT_FAILURE;
    }
    catch(const char* msg)
    {
        Error out(getProcessLogger());
        out << msg;
        status = EXIT_FAILURE;
    }
    catch(...)
    {
        Error out(getProcessLogger());
        out << "unknown exception";
        status = EXIT_FAILURE;
    }

    //
    // Don't want any new interrupt and at this point (post-run),
    // it would not make sense to release a held signal to run
    // shutdown or destroy.
    //
    if(IceInternal::_signalPolicy == HandleSignals)
    {
        ignoreInterrupt();
    }

    {
        IceUtil::Mutex::Lock lock(*IceInternal::mutex);
        while(IceInternal::_callbackInProgress)
        {
            IceInternal::_condVar->wait(lock);
        }
        if(IceInternal::_destroyed)
        {
            IceInternal::_communicator = 0;
        }
        else
        {
            IceInternal::_destroyed = true;
            //
            // And _communicator != 0, meaning will be destroyed
            // next, _destroyed = true also ensures that any
            // remaining callback won't do anything
            //
        }
        IceInternal::_application = 0;
    }

    if(IceInternal::_communicator != 0)
    {  
        try
        {
            IceInternal::_communicator->destroy();
        }
        catch(const std::exception& ex)
        {
            Error out(getProcessLogger());
            out << ex;
            status = EXIT_FAILURE;
        }
        catch(...)
        {
            Error out(getProcessLogger());
            out << "unknown exception";
            status = EXIT_FAILURE;
        }
        IceInternal::_communicator = 0;
    }

    return status;
}
