// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/LoggerI.h>
#include <Ice/LoggerUtil.h>
#include <Ice/ArgVector.h>

#ifdef _WIN32
const DWORD SIGHUP = CTRL_LOGOFF_EVENT;
#else
#   include <csignal>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceUtil;
using namespace IceUtilInternal;

//
// static member initialization
//
IceUtil::Mutex Ice::Application::_mutex;
IceUtil::Cond Ice::Application::_condVar;

bool Ice::Application::_callbackInProgress = false;
bool Ice::Application::_destroyed = false;
bool Ice::Application::_interrupted = false;

string Ice::Application::_appName;
Ice::CommunicatorPtr Ice::Application::_communicator;
Ice::SignalPolicy Ice::Application::_signalPolicy = ICE_ENUM(SignalPolicy, HandleSignals);
Ice::Application* Ice::Application::_application = 0;

namespace
{

//
// Variables than can change while run() and communicator->destroy() are running!
//
bool _released = true;
CtrlCHandlerCallback _previousCallback = ICE_NULLPTR;

//
// Variables that are immutable during run() and until communicator->destroy() has returned;
// before and after run(), and once communicator->destroy() has returned, we assume that
// only the main thread and CtrlCHandler threads are running.
//
CtrlCHandler* _ctrlCHandler = 0;
bool _nohup = false;

}

Ice::Application::Application(SignalPolicy signalPolicy)
{
    Ice::Application::_signalPolicy = signalPolicy;
}

Ice::Application::~Application()
{
}

int
Ice::Application::main(int argc, const char* const argv[], ICE_CONFIG_FILE_STRING configFile, int version)
{
    _appName = "";
    if(argc > 0)
    {
        _appName = argv[0];
    }

    if(argc > 0 && argv[0] && ICE_DYNAMIC_CAST(LoggerI, getProcessLogger()))
    {
        setProcessLogger(ICE_MAKE_SHARED(LoggerI, argv[0], "", true));
    }

    InitializationData initData;
#ifndef ICE_CPP11_MAPPING
    if(configFile)
#endif
    {
        try
        {
            initData.properties = createProperties();
            initData.properties->load(configFile);
        }
        catch(const Ice::Exception& ex)
        {
            Error out(getProcessLogger());
            out << ex;
            return EXIT_FAILURE;
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
    return main(argc, argv, initData, version);
}

#ifdef _WIN32
int
Ice::Application::main(int argc, const wchar_t* const argv[], const Ice::InitializationData& initData, int version)
{
    //
    // On Windows the given wchar_t* strings are UTF16 and therefore
    // needs to be converted to native narrow string encoding.
    //
    return main(argsToStringSeq(argc, argv), initData, version);
}

int
Ice::Application::main(int argc, const wchar_t* const argv[], ICE_CONFIG_FILE_STRING config, int version)
{
    return main(argsToStringSeq(argc, argv), config, version);
}
#endif

int
Ice::Application::main(int argc, const char* const argv[], const InitializationData& initializationData, int version)
{
    if(argc > 0 && argv[0] && ICE_DYNAMIC_CAST(LoggerI, getProcessLogger()))
    {
        const bool convert = initializationData.properties ?
            initializationData.properties->getPropertyAsIntWithDefault("Ice.LogStdErr.Convert", 1) > 0 &&
            initializationData.properties->getProperty("Ice.StdErr").empty() : true;
        setProcessLogger(ICE_MAKE_SHARED(LoggerI, argv[0], "", convert));
    }

    if(_communicator != 0)
    {
        Error out(getProcessLogger());
        out << "only one instance of the Application class can be used";
        return EXIT_FAILURE;
    }
    int status;

    ArgVector av(argc, argv); // copy args

    //
    // We parse the properties here to extract Ice.ProgramName.
    //
    InitializationData initData = initializationData;
    try
    {
        initData.properties = createProperties(av.argc, av.argv, initData.properties);
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << ex;
        return EXIT_FAILURE;
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
    _appName = initData.properties->getPropertyWithDefault("Ice.ProgramName", _appName);

    //
    // Used by destroyOnInterruptCallback and shutdownOnInterruptCallback.
    //
    _nohup = initData.properties->getPropertyAsInt("Ice.Nohup") > 0;

    _application = this;

    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        try
        {
            //
            // The ctrlCHandler must be created before starting any thread, in particular
            // before initializing the communicator.
            //
            CtrlCHandler ctrCHandler;
            _ctrlCHandler = &ctrCHandler;

            status = doMain(av.argc, av.argv, initData, version);

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
        status = doMain(av.argc, av.argv, initData, version);
    }

    return status;
}

int
Ice::Application::main(const StringSeq& args, const InitializationData& initData, int version)
{
    ArgVector av(args);
    return main(av.argc, av.argv, initData, version);
}

int
Ice::Application::main(const StringSeq& args, ICE_CONFIG_FILE_STRING configFile, int version)
{
    ArgVector av(args);
    return main(av.argc, av.argv, configFile, version);
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(!_released)
            {
                _released = true;
                _condVar.signal();
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(!_released)
            {
                _released = true;
                _condVar.signal();
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(!_released)
            {
                _released = true;
                _condVar.signal();
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(!_released)
            {
                _released = true;
                _condVar.signal();
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(_released)
            {
                _released = false;
                _previousCallback = _ctrlCHandler->setCallback(holdInterruptCallback);
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        if(_ctrlCHandler != 0)
        {
            Mutex::Lock lock(_mutex); // we serialize all the interrupt-setting
            if(!_released)
            {
                //
                // Note that it's very possible no signal is held;
                // in this case the callback is just replaced and
                // setting _released to true and signalling _condVar
                // do no harm.
                //

                _released = true;
                _ctrlCHandler->setCallback(_previousCallback);
                _condVar.signal();
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
    Mutex::Lock lock(_mutex);
    return _interrupted;
}

int
Ice::Application::doMain(int argc, char* argv[], const InitializationData& initData, int version)
{
    int status;

    try
    {
        _interrupted = false;

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

        _communicator = initialize(argc, argv, initData, version);
        _destroyed = false;

        //
        // The default is to destroy when a signal is received.
        //
        if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
        {
            destroyOnInterrupt();
        }

        status = run(argc, argv);
    }
    catch(const Ice::Exception& ex)
    {
        Error out(getProcessLogger());
        out << ex;
        status = EXIT_FAILURE;
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
    if(_signalPolicy == ICE_ENUM(SignalPolicy, HandleSignals))
    {
        ignoreInterrupt();
    }

    {
        Mutex::Lock lock(_mutex);
        while(_callbackInProgress)
        {
            _condVar.wait(lock);
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
        _communicator->destroy();
        _communicator = 0;
    }

    return status;
}

//
// CtrlCHandler callbacks.
//

void
Ice::Application::holdInterruptCallback(int signal)
{
    CtrlCHandlerCallback callback = ICE_NULLPTR;
    {
        Mutex::Lock lock(_mutex);
        while(!_released)
        {
            _condVar.wait(lock);
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

    if(callback)
    {
        callback(signal);
    }
}

void
Ice::Application::destroyOnInterruptCallback(int signal)
{
    {
        Mutex::Lock lock(_mutex);
        if(_destroyed)
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

        assert(!_callbackInProgress);
        _callbackInProgress = true;
        _interrupted = true;
        _destroyed = true;
    }

    assert(_communicator != 0);
    _communicator->destroy();

    {
        Mutex::Lock lock(_mutex);
        _callbackInProgress = false;
    }
    _condVar.signal();
}

void
Ice::Application::shutdownOnInterruptCallback(int signal)
{
    {
        Mutex::Lock lock(_mutex);
        if(_destroyed)
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

        assert(!_callbackInProgress);
        _callbackInProgress = true;
        _interrupted = true;
    }

    assert(_communicator != 0);
    _communicator->shutdown();

    {
        Mutex::Lock lock(_mutex);
        _callbackInProgress = false;
    }
    _condVar.signal();
}

void
Ice::Application::callbackOnInterruptCallback(int signal)
{
    {
        Mutex::Lock lock(_mutex);
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
        out << "(while interrupting in response to signal " << signal << "): Ice::Exception: " << ex;
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
        Mutex::Lock lock(_mutex);
        _callbackInProgress = false;
    }
    _condVar.signal();
}
