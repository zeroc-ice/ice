// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/UniquePtr.h>

using namespace std;
using namespace Ice;
using namespace IceUtil;
using namespace IceUtilInternal;

//
// static initializations.
//
Mutex* IceInternal::Application::mutex = 0;

bool IceInternal::Application::_callbackInProgress = false;
bool IceInternal::Application::_destroyed = false;
bool IceInternal::Application::_interrupted = false;

string IceInternal::Application::_appName;
CommunicatorPtr IceInternal::Application::_communicator;
SignalPolicy IceInternal::Application::_signalPolicy = HandleSignals;
Cond* IceInternal::Application::_condVar;
Application* IceInternal::Application::_application;
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
        IceInternal::Application::mutex = new IceUtil::Mutex;
        IceInternal::Application::_condVar = new Cond();
    }

    ~Init()
    {
        delete IceInternal::Application::mutex;
        IceInternal::Application::mutex = 0;
        delete IceInternal::Application::_condVar;
        IceInternal::Application::_condVar = 0;
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
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        while(!_released)
        {
            IceInternal::Application::_condVar->wait(lock);
        }

        if(IceInternal::Application::_destroyed)
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
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        if(IceInternal::Application::_destroyed)
        {
            //
            // Being destroyed by main thread
            //
            return;
        }
        if(_nohup && signal == static_cast<int>(SIGHUP))
        {
            return;
        }

        assert(!IceInternal::Application::_callbackInProgress);
        IceInternal::Application::_callbackInProgress = true;
        IceInternal::Application::_interrupted = true;
        IceInternal::Application::_destroyed = true;
    }

    try
    {
        assert(IceInternal::Application::_communicator != 0);
        IceInternal::Application::_communicator->destroy();
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
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        IceInternal::Application::_callbackInProgress = false;
    }
    IceInternal::Application::_condVar->signal();
}

static void
shutdownOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        if(IceInternal::Application::_destroyed)
        {
            //
            // Being destroyed by main thread
            //
            return;
        }
        if(_nohup && signal == static_cast<int>(SIGHUP))
        {
            return;
        }

        assert(!IceInternal::Application::_callbackInProgress);
        IceInternal::Application::_callbackInProgress = true;
        IceInternal::Application::_interrupted = true;
    }

    try
    {
        assert(IceInternal::Application::_communicator != 0);
        IceInternal::Application::_communicator->shutdown();
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
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        IceInternal::Application::_callbackInProgress = false;
    }
    IceInternal::Application::_condVar->signal();
}

static void
callbackOnInterruptCallback(int signal)
{
    {
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        if(IceInternal::Application::_destroyed)
        {
            //
            // Being destroyed by main thread
            //
            return;
        }
        // For SIGHUP the user callback is always called. It can
        // decide what to do.
        assert(!IceInternal::Application::_callbackInProgress);
        IceInternal::Application::_callbackInProgress = true;
        IceInternal::Application::_interrupted = true;
    }

    try
    {
        assert(IceInternal::Application::_application != 0);
        IceInternal::Application::_application->interruptCallback(signal);
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
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        IceInternal::Application::_callbackInProgress = false;
    }
    IceInternal::Application::_condVar->signal();
}

Ice::Application::Application(SignalPolicy signalPolicy)
{
    IceInternal::Application::_signalPolicy = signalPolicy;
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, char* argv[], const char* configFile)
{
    //
    // We don't call the main below to avoid a deprecated warning
    //

    IceInternal::Application::_appName = "";
    if(argc > 0)
    {
        IceInternal::Application::_appName = argv[0];
    }

    if(argc > 0 && argv[0] && ICE_DYNAMIC_CAST(LoggerI, getProcessLogger()))
    {
        setProcessLogger(ICE_MAKE_SHARED(LoggerI, argv[0], "", true));
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

#ifdef _WIN32

int
Ice::Application::main(int argc, wchar_t* argv[], const char* config)
{
    return main(argsToStringSeq(argc, argv), config);
}

int
Ice::Application::main(int argc, wchar_t* argv[], const Ice::InitializationData& initData)
{
    //
    // On Windows the given wchar_t* strings are UTF16 and therefore
    // needs to be converted to native narow string encoding.
    //
    return main(argsToStringSeq(argc, argv), initData);
}

#endif

int
Ice::Application::main(int argc, char* argv[], const InitializationData& initializationData)
{
    if(argc > 0 && argv[0] && ICE_DYNAMIC_CAST(LoggerI, getProcessLogger()))
    {
        const bool convert = initializationData.properties ?
                initializationData.properties->getPropertyAsIntWithDefault("Ice.LogStdErr.Convert", 1) > 0 &&
                initializationData.properties->getProperty("Ice.StdErr").empty() : true;
        setProcessLogger(ICE_MAKE_SHARED(LoggerI, argv[0], "", convert));
    }

    if(IceInternal::Application::_communicator != 0)
    {
        Error out(getProcessLogger());
        out << "only one instance of the Application class can be used";
        return EXIT_FAILURE;
    }
    int status;

    //
    // We parse the properties here to extract Ice.ProgramName.
    //
    InitializationData initData = initializationData;
    try
    {
        initData.properties = createProperties(argc, argv, initData.properties);
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
    IceInternal::Application::_appName = initData.properties->getPropertyWithDefault("Ice.ProgramName",
                                                                                 IceInternal::Application::_appName);

    //
    // Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
    //
    _nohup = initData.properties->getPropertyAsInt("Ice.Nohup") > 0;

    IceInternal::Application::_application = this;

    if(IceInternal::Application::_signalPolicy == HandleSignals)
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
Ice::Application::main(int argc, char* const argv[], const char* configFile)
{
    ArgVector av(argc, argv);
    return main(av.argc, av.argv, configFile);
}

int
Ice::Application::main(int argc, char* const argv[], const Ice::InitializationData& initData)
{
    ArgVector av(argc, argv);
    return main(av.argc, av.argv, initData);
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
    return IceInternal::Application::_appName.c_str();
}

CommunicatorPtr
Ice::Application::communicator()
{
    return IceInternal::Application::_communicator;
}

void
Ice::Application::destroyOnInterrupt()
{
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::Application::_condVar->signal();
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::Application::_condVar->signal();
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::Application::_condVar->signal();
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
            if(_ctrlCHandler->getCallback() == holdInterruptCallback)
            {
                _released = true;
                IceInternal::Application::_condVar->signal();
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        if(_ctrlCHandler != 0)
        {
            IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex); // we serialize all the interrupt-setting
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
                IceInternal::Application::_condVar->signal();
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
    IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
    return IceInternal::Application::_interrupted;
}

int
Ice::Application::doMain(int argc, char* argv[], const InitializationData& initData)
{
    int status;

    try
    {
        IceInternal::Application::_interrupted = false;

        //
        // If the process logger is the default logger, we now replace it with a
        // a logger which is using the program name for the prefix.
        //
        if(initData.properties->getProperty("Ice.ProgramName") != "" && ICE_DYNAMIC_CAST(LoggerI, getProcessLogger()))
        {
            const bool convert =
                initData.properties->getPropertyAsIntWithDefault("Ice.LogStdErr.Convert", 1) > 0 &&
                initData.properties->getProperty("Ice.StdErr").empty();

            setProcessLogger(ICE_MAKE_SHARED(LoggerI, initData.properties->getProperty("Ice.ProgramName"), "", convert));
        }

        IceInternal::Application::_communicator = initialize(argc, argv, initData);
        IceInternal::Application::_destroyed = false;

        //
        // The default is to destroy when a signal is received.
        //
        if(IceInternal::Application::_signalPolicy == HandleSignals)
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
    if(IceInternal::Application::_signalPolicy == HandleSignals)
    {
        ignoreInterrupt();
    }

    {
        IceUtil::Mutex::Lock lock(*IceInternal::Application::mutex);
        while(IceInternal::Application::_callbackInProgress)
        {
            IceInternal::Application::_condVar->wait(lock);
        }
        if(IceInternal::Application::_destroyed)
        {
            IceInternal::Application::_communicator = 0;
        }
        else
        {
            IceInternal::Application::_destroyed = true;
            //
            // And _communicator != 0, meaning will be destroyed
            // next, _destroyed = true also ensures that any
            // remaining callback won't do anything
            //
        }
        IceInternal::Application::_application = 0;
    }

    if(IceInternal::Application::_communicator != 0)
    {
        try
        {
            IceInternal::Application::_communicator->destroy();
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
        IceInternal::Application::_communicator = 0;
    }

    return status;
}
