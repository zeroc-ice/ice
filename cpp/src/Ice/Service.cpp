// **********************************************************************
//
// Copyright (c) 2003
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

#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <Ice/Service.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

#ifdef _WIN32
#   include <Ice/EventLoggerI.h>
#else
#   include <Ice/Logger.h>
#   include <Ice/Network.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <csignal>
#endif

using namespace std;

Ice::Service* Ice::Service::_instance = 0;
static IceUtil::CtrlCHandler* _ctrlCHandler = 0;

//
// Callback for IceUtil::CtrlCHandler.
//
static void
ctrlCHandlerCallback(int sig)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != 0);
    service->handleInterrupt(sig);
}

#ifdef _WIN32
extern "C"
{

//
// Main function for Win32 service.
//
void
Ice_Service_ServiceMain(DWORD argc, LPTSTR* argv)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != 0);
    service->serviceMain(argc, argv);
}

//
// Win32 service control handler.
//
void
Ice_Service_CtrlHandler(DWORD ctrl)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != 0);
    service->control(ctrl);
}

}

namespace Ice
{

class ServiceStatusThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ServiceStatusThread(SERVICE_STATUS_HANDLE, SERVICE_STATUS*);

    virtual void run();

    void stop(DWORD, DWORD);

private:

    SERVICE_STATUS_HANDLE _handle;
    SERVICE_STATUS* _status;
    bool _stopped;
};
typedef IceUtil::Handle<ServiceStatusThread> ServiceStatusThreadPtr;

}
#endif

Ice::Service::Service()
{
    assert(_instance == 0);
    _instance = this;
    _nohup = true;

#ifdef _WIN32
    //
    // Check for Windows 9x/ME.
    //
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);
    _win9x = (ver.dwPlatformId != VER_PLATFORM_WIN32_NT);
    _service = false;
#endif
}

Ice::Service::~Service()
{
    _instance = 0;
    delete _ctrlCHandler;
}

bool
Ice::Service::shutdown()
{
    if(_communicator)
    {
        _communicator->shutdown();
    }
    return true;
}

void
Ice::Service::interrupt()
{
    shutdown();
}

int
Ice::Service::main(int argc, char* argv[])
{
    _prog = argv[0];

    int status;
#ifdef _WIN32
    if(checkService(argc, argv, status))
    {
        return status;
    }
#else
    if(checkDaemon(argc, argv, status))
    {
        return status;
    }
#endif

    try
    {
        //
        // Create the CtrlCHandler after any potential forking so that signals
        // are initialized properly. We do this before initializing the
        // communicator because we need to ensure that this is done before any
        // additional threads are created.
        //
        _ctrlCHandler = new IceUtil::CtrlCHandler;

        //
        // Initialize the communicator.
        //
        _communicator = initializeCommunicator(argc, argv);

        //
        // Use the configured logger.
        //
        _logger = _communicator->getLogger();

        //
        // Determines whether we ignore SIGHUP/CTRL_LOGOFF_EVENT.
        //
        _nohup = _communicator->getProperties()->getPropertyAsIntWithDefault("Ice.Nohup", 1) > 0;

        //
        // Start the service.
        //
        status = EXIT_FAILURE;
        if(start(argc, argv))
        {
            //
            // Wait for service shutdown.
            //
            waitForShutdown();

            //
            // Stop the service.
            //
            if(stop())
            {
                status = EXIT_SUCCESS;
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << "service caught unhandled Ice exception:" << endl << ex;
        error(ostr.str());
    }
    catch(...)
    {
        error("service caught unhandled C++ exception");
    }

    try
    {
        _communicator->destroy();
    }
    catch(...)
    {
    }

    return status;
}

Ice::CommunicatorPtr
Ice::Service::communicator() const
{
    return _communicator;
}

Ice::Service*
Ice::Service::instance()
{
    return _instance;
}

void
Ice::Service::handleInterrupt(int sig)
{
#ifdef _WIN32
    if(_nohup && sig == CTRL_LOGOFF_EVENT)
    {
        return;
    }
#else
    if(_nohup && sig == SIGHUP)
    {
        return;
    }
#endif

    interrupt();
}

void
Ice::Service::waitForShutdown()
{
    if(_communicator)
    {
        enableInterrupt();
        _communicator->waitForShutdown();
        disableInterrupt();
    }
}

bool
Ice::Service::stop()
{
    return true;
}

Ice::CommunicatorPtr
Ice::Service::initializeCommunicator(int& argc, char* argv[])
{
    return Ice::initialize(argc, argv);
}

void
Ice::Service::enableInterrupt()
{
    _ctrlCHandler->setCallback(ctrlCHandlerCallback);
}

void
Ice::Service::disableInterrupt()
{
    _ctrlCHandler->setCallback(0);
}

void
Ice::Service::syserror(const std::string& msg) const
{
    string errmsg;
#ifdef _WIN32
    int err = GetLastError();
    if(err < WSABASEERR)
    {
        LPVOID lpMsgBuf = 0;
        DWORD ok = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
                                 FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL,
                                 err,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                 (LPTSTR)&lpMsgBuf,
                                 0,
                                 NULL);
        if(ok)
        {
            LPCTSTR str = (LPCTSTR)lpMsgBuf;
            assert(str && strlen((const char*)str) > 0);
            errmsg = (const char*)str;
            LocalFree(lpMsgBuf);
        }
    }
#else
    int err = errno;
    errmsg = strerror(err);
#endif
    if(_logger)
    {
        ostringstream ostr;
        if(!msg.empty())
        {
            ostr << msg << endl;
        }
        if(!errmsg.empty())
        {
            ostr << errmsg;
        }
        _logger->error(ostr.str());
    }
    else
    {
        cerr << _prog << ": ";
        if(!msg.empty())
        {
            cerr << msg << endl;
        }
        if(!errmsg.empty())
        {
            cerr << errmsg;
        }
    }
}

void
Ice::Service::error(const std::string& msg) const
{
    if(_logger)
    {
        _logger->error(msg);
    }
    else
    {
        cerr << _prog << ": error: " << msg << endl;
    }
}

void
Ice::Service::warning(const std::string& msg) const
{
    if(_logger)
    {
        _logger->warning(msg);
    }
    else
    {
        cerr << _prog << ": warning: " << msg << endl;
    }
}

void
Ice::Service::trace(const std::string& msg) const
{
    if(_logger)
    {
        _logger->trace("", msg);
    }
    else
    {
        cerr << msg << endl;
    }
}

#ifdef _WIN32

bool
Ice::Service::win9x() const
{
    return _win9x;
}

bool
Ice::Service::checkService(int argc, char* argv[], int& status)
{
    if(_win9x)
    {
        return false;
    }

    string name;

    //
    // First check for the --service option.
    //
    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--service") == 0)
        {
            if(idx + 1 >= argc)
            {
                error("service name argument expected for `" + string(argv[idx]) + "'");
                status = EXIT_FAILURE;
                return true;
            }

            name = argv[idx + 1];

            for(int i = idx; i + 2 < argc; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;

            _service = true;
        }
        else
        {
            ++idx;
        }
    }

    //
    // Next check for service control options.
    //
    string op;
    idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--install") == 0 ||
           strcmp(argv[idx], "--uninstall") == 0 ||
           strcmp(argv[idx], "--start") == 0 ||
           strcmp(argv[idx], "--stop") == 0)
        {
            if(_service)
            {
                error("cannot specify `--service' and `" + string(argv[idx]) + "'");
                status = EXIT_FAILURE;
                return true;
            }

            if(!op.empty())
            {
                error("cannot specify `" + op + "' and `" + string(argv[idx]) + "'");
                status = EXIT_FAILURE;
                return true;
            }

            if(idx + 1 >= argc)
            {
                error("service name argument expected for `" + string(argv[idx]) + "'");
                status = EXIT_FAILURE;
                return true;
            }

            op = argv[idx];
            name = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else
        {
            ++idx;
        }
    }

    if(!op.empty())
    {
        if(op == "--install")
        {
            status = installService(name, argc, argv);
        }
        else if(op == "--uninstall")
        {
            status = uninstallService(name, argc, argv);
        }
        else if(op == "--start")
        {
            status = startService(name, argc, argv);
        }
        else
        {
            assert(op == "--stop");
            status = stopService(name, argc, argv);
        }
        return true;
    }

    //
    // Run as a service.
    //
    if(_service)
    {
        //
        // When running as a service, we need an event logger in order to report
        // failures that occur prior to initializing a communicator. After we have
        // a communicator, we can use the configured logger instead.
        //
        // We postpone the initialization of the communicator until serviceMain so
        // that we can incorporate the executable's arguments and the service's
        // arguments into one vector.
        //
        try
        {
            _logger = new EventLoggerI(name);
        }
        catch(const IceUtil::Exception& ex)
        {
            ostringstream ostr;
            ostr << ex;
            error("unable to create EventLogger:\n" + ostr.str());
            status = EXIT_FAILURE;
            return true;
        }

        //
        // Arguments passed to the executable are not passed to the service's main function,
        // so save them now and serviceMain will merge them later.
        //
        for(idx = 1; idx < argc; ++idx)
        {
            _serviceArgs.push_back(argv[idx]);
        }

        SERVICE_TABLE_ENTRY ste[] =
        {
            { const_cast<char*>(name.c_str()), (LPSERVICE_MAIN_FUNCTIONA)Ice_Service_ServiceMain },
            { NULL, NULL },
        };

        if(!StartServiceCtrlDispatcher(ste))
        {
            syserror("unable to start service control dispatcher");
            status = EXIT_FAILURE;
            return true;
        }

        status = EXIT_SUCCESS;
        return true;
    }

    return false;
}

int
Ice::Service::installService(const std::string& name, int argc, char* argv[])
{
    string display;
    string executable;
    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--display") == 0)
        {
            if(idx + 1 >= argc)
            {
                error("argument expected for `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            display = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else if(strcmp(argv[idx], "--executable") == 0)
        {
            if(idx + 1 >= argc)
            {
                error("argument expected for `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            executable = argv[idx + 1];

            for(int i = idx ; i + 2 < argc ; ++i)
            {
                argv[i] = argv[i + 2];
            }
            argc -= 2;
        }
        else
        {
            ++idx;
        }
    }

    if(display.empty())
    {
        display = name;
    }

    if(executable.empty())
    {
        //
        // Use this executable if none is specified.
        //
        char buf[_MAX_PATH];
        if(GetModuleFileName(NULL, buf, _MAX_PATH) == 0)
        {
            error("unable to obtain file name of executable");
            return EXIT_FAILURE;
        }
        executable = buf;
    }

    //
    // Compose service command line. The executable and any arguments must
    // be enclosed in quotes if they contain whitespace.
    //
    string command;
    if(executable.find(' ') != string::npos)
    {
        command.push_back('"');
        command.append(executable);
        command.push_back('"');
    }
    else
    {
        command = executable;
    }
    command.append(" --service ");
    if(name.find(' ') != string::npos)
    {
        command.push_back('"');
        command.append(name);
        command.push_back('"');
    }
    else
    {
        command.append(name);
    }
    for(idx = 1; idx < argc; ++idx)
    {
        command.push_back(' ');

        string arg = argv[idx];
        if(arg.find_first_of(" \t\n\r") != string::npos)
        {
            command.push_back('"');
            command.append(arg);
            command.push_back('"');
        }
        else
        {
            command.append(arg);
        }
    }

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(hSCM == NULL)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }
    SC_HANDLE hService = CreateService(
        hSCM,
        name.c_str(),
        display.c_str(),
        SC_MANAGER_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        command.c_str(),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL);

    if(hService == NULL)
    {
        syserror("unable to install service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);

    return EXIT_SUCCESS;
}

int
Ice::Service::uninstallService(const std::string& name, int argc, char* argv[])
{
    if(argc > 1)
    {
        error("unknown option `" + string(argv[1]) + "'");
        return EXIT_FAILURE;
    }

    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(hSCM == NULL)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SC_MANAGER_ALL_ACCESS);
    if(hService == NULL)
    {
        syserror("unable to open service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    BOOL b = DeleteService(hService);

    if(!b)
    {
        syserror("unable to uninstall service `" + name + "'");
        CloseServiceHandle(hSCM);
        CloseServiceHandle(hService);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);

    return EXIT_SUCCESS;
}

int
Ice::Service::startService(const std::string& name, int argc, char* argv[])
{
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(hSCM == NULL)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SC_MANAGER_ALL_ACCESS);
    if(hService == NULL)
    {
        syserror("unable to open service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    //
    // Create argument vector.
    //
    LPCSTR* args = new LPCSTR[argc];
    args[0] = const_cast<char*>(name.c_str());
    for(int i = 1; i < argc; ++i)
    {
        args[i] = argv[i];
    }

    BOOL b = StartService(hService, argc, args);

    delete[] args;

    if(!b)
    {
        syserror("unable to start service `" + name + "'");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    SERVICE_STATUS status;
    DWORD oldCheckPoint;
    DWORD startTickCount;
    DWORD waitTime;

    trace("Service start pending.");

    //
    // Get the initial status of the service.
    //
    if(!QueryServiceStatus(hService, &status))
    {
        syserror("unable to query status of service `" + name + "'");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    //
    // Save the tick count and initial checkpoint.
    //
    startTickCount = GetTickCount();
    oldCheckPoint = status.dwCheckPoint;

    //
    // Loop until the service is started or an error is detected.
    //
    while(status.dwCurrentState == SERVICE_START_PENDING)
    {
        //
        // Do not wait longer than the wait hint. A good interval is
        // one tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.
        //

        waitTime = status.dwWaitHint / 10;

        if(waitTime < 1000)
        {
            waitTime = 1000;
        }
        else if(waitTime > 10000)
        {
            waitTime = 10000;
        }

        Sleep(waitTime);

        //
        // Check the status again.
        //
        if(!QueryServiceStatus(hService, &status))
        {
            break;
        }

        if(status.dwCheckPoint > oldCheckPoint)
        {
            //
            // The service is making progress.
            //
            startTickCount = GetTickCount();
            oldCheckPoint = status.dwCheckPoint;
        }
        else
        {
            if(GetTickCount() - startTickCount > status.dwWaitHint)
            {
                //
                // No progress made within the wait hint.
                //
                break;
            }
        }
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    if(status.dwCurrentState == SERVICE_RUNNING)
    {
        trace("Service is running.");
    }
    else
    {
        ostringstream ostr;
        ostr << "Service failed to start." << endl
             << "  Current state: " << status.dwCurrentState << endl
             << "  Exit code: " << status.dwWin32ExitCode << endl
             << "  Service specific exit code: " << status.dwServiceSpecificExitCode << endl
             << "  Check point: " << status.dwCheckPoint << endl
             << "  Wait hint: " << status.dwWaitHint;
        trace(ostr.str());
    }

    return EXIT_SUCCESS;
}

int
Ice::Service::stopService(const std::string& name, int argc, char* argv[])
{
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if(hSCM == NULL)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SC_MANAGER_ALL_ACCESS);
    if(hService == NULL)
    {
        syserror("unable to open service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    SERVICE_STATUS status;
    BOOL b = ControlService(hService, SERVICE_CONTROL_STOP, &status);

    if(!b)
    {
        syserror("unable to stop service `" + name + "'");
        CloseServiceHandle(hSCM);
        CloseServiceHandle(hService);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);

    ostringstream ostr;
    ostr << "Stop request sent to service." << endl
         << "  Current state: " << status.dwCurrentState << endl
         << "  Exit code: " << status.dwWin32ExitCode << endl
         << "  Service specific exit code: " << status.dwServiceSpecificExitCode << endl
         << "  Check point: " << status.dwCheckPoint << endl
         << "  Wait hint: " << status.dwWaitHint;
    trace(ostr.str());

    return EXIT_SUCCESS;
}

void
Ice::Service::serviceMain(int argc, char* argv[])
{
    _ctrlCHandler = new IceUtil::CtrlCHandler;

    //
    // Initialize the service status.
    //
    _status.dwServiceType = SERVICE_WIN32;
    _status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    _status.dwWin32ExitCode = 0;
    _status.dwServiceSpecificExitCode = 0;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;

    //
    // Register the control handler function.
    //
    _statusHandle = RegisterServiceCtrlHandler(argv[0], (LPHANDLER_FUNCTION)Ice_Service_CtrlHandler);
    if(_statusHandle == (SERVICE_STATUS_HANDLE)0)
    {
        syserror("unable to register service control handler");
        return;
    }

    //
    // Start a thread to periodically update the service's status with the
    // service control manager (SCM). The SCM must receive periodic updates
    // otherwise it assumes that initialization failed and terminates the
    // service.
    //
    _status.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatusThreadPtr statusThread = new ServiceStatusThread(_statusHandle, &_status);
    statusThread->start();

    //
    // Merge the executable's arguments with the service's arguments.
    //
    char** args = new char*[_serviceArgs.size() + argc];
    args[0] = argv[0];
    int i = 1;
    for(vector<string>::iterator p = _serviceArgs.begin(); p != _serviceArgs.end(); ++p)
    {
        args[i++] = const_cast<char*>(p->c_str());
    }
    for(int j = 1; j < argc; ++j)
    {
        args[i++] = argv[j];
    }
    argc += _serviceArgs.size();

    //
    // If we can't initialize a communicator, then stop immediately.
    //
    try
    {
        _communicator = initializeCommunicator(argc, args);
    }
    catch(const Ice::Exception& ex)
    {
        // TODO: Enable delete when we figure out why it can cause a crash.
        //delete[] args;
        ostringstream ostr;
        ostr << ex;
        error(ostr.str());
        statusThread->stop(SERVICE_STOPPED, EXIT_FAILURE);
        statusThread->getThreadControl().join();
        return;
    }

    //
    // Use the configured logger.
    //
    _logger = _communicator->getLogger();

    //
    // Determines whether we ignore SIGHUP/CTRL_LOGOFF_EVENT.
    //
    _nohup = _communicator->getProperties()->getPropertyAsIntWithDefault("Ice.Nohup", 1) > 0;

    DWORD status = EXIT_FAILURE;
    try
    {
        if(start(argc, args))
        {
            trace("Service started successfully.");

            //
            // Stop the status thread and set our current status to running.
            //
            statusThread->stop(SERVICE_RUNNING, NO_ERROR);
            statusThread->getThreadControl().join();
            statusThread = 0;

            //
            // Wait for the service to be shut down.
            //
            waitForShutdown();

            //
            // Notify the service control manager that a stop is pending.
            //
            _status.dwCurrentState = SERVICE_STOP_PENDING;
            statusThread = new ServiceStatusThread(_statusHandle, &_status);
            statusThread->start();

            //
            // Give the service a chance to clean up.
            //
            if(stop())
            {
                status = EXIT_SUCCESS;
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << "service caught unhandled Ice exception:" << endl << ex;
        error(ostr.str());
    }
    catch(...)
    {
        error("service caught unhandled C++ exception");
    }

    // TODO: Enable delete when we figure out why it can cause a crash.
    //delete[] args;

    try
    {
        _communicator->destroy();
    }
    catch(...)
    {
    }

    if(statusThread)
    {
        statusThread->stop(SERVICE_STOPPED, status);
        statusThread->getThreadControl().join();
    }
}

void
Ice::Service::control(int ctrl)
{
    switch(ctrl)
    {
        case SERVICE_CONTROL_INTERROGATE:
        {
            SERVICE_STATUS status = _status; // TODO: Small risk of race with ServiceStatusThread
            if(!SetServiceStatus(_statusHandle, &status))
            {
                syserror("unable to set service status");
            }
            break;
        }
        case SERVICE_CONTROL_STOP:
        {
            //
            // Shut down the service. The serviceMain method will update the service status.
            //
            shutdown();
            break;
        }
        default:
        {
            ostringstream ostr;
            ostr << "unrecognized service control code " << ctrl;
            error(ostr.str());
            break;
        }
    }
}

Ice::ServiceStatusThread::ServiceStatusThread(SERVICE_STATUS_HANDLE handle, SERVICE_STATUS* status) :
    _handle(handle), _status(status), _stopped(false)
{
}

void
Ice::ServiceStatusThread::run()
{
    Lock sync(*this);

    IceUtil::Time delay = IceUtil::Time::milliSeconds(500);
    _status->dwWaitHint = 1000;

    while(!_stopped)
    {
        if(!SetServiceStatus(_handle, _status))
        {
            return;
        }

        timedWait(delay);

        _status->dwCheckPoint++;
    }

    _status->dwCheckPoint = 0;
    _status->dwWaitHint = 0;
    SetServiceStatus(_handle, _status);
}

void
Ice::ServiceStatusThread::stop(DWORD state, DWORD exitCode)
{
    Lock sync(*this);
    _status->dwCurrentState = state;
    _status->dwWin32ExitCode = exitCode;
    _stopped = true;
    notify();
}

#else

bool
Ice::Service::checkDaemon(int argc, char* argv[], int& status)
{
    //
    // Check for --daemon, --noclose and --nochdir.
    //
    bool daemonize = false;
    bool noClose = false;
    bool noChdir = false;
    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--daemon") == 0)
        {
            for(int i = idx; i + 1 < argc; ++i)
            {
                argv[i] = argv[i + 1];
            }
            argc -= 1;

            daemonize = true;
        }
        else if(strcmp(argv[idx], "--noclose") == 0)
        {
            for(int i = idx; i + 1 < argc; ++i)
            {
                argv[i] = argv[i + 1];
            }
            argc -= 1;

            noClose = true;
        }
        else if(strcmp(argv[idx], "--nochdir") == 0)
        {
            for(int i = idx; i + 1 < argc; ++i)
            {
                argv[i] = argv[i + 1];
            }
            argc -= 1;

            noChdir = true;
        }
        else
        {
            ++idx;
        }
    }

    if(noClose && !daemonize)
    {
        cerr << argv[0] << ": --noclose must be used with --daemon" << endl;
        status = EXIT_FAILURE;
        return true;
    }

    if(noChdir && !daemonize)
    {
        cerr << argv[0] << ": --nochdir must be used with --daemon" << endl;
        status = EXIT_FAILURE;
        return true;
    }

    if(!daemonize)
    {
        return false;
    }

    //
    // Create a pipe that is used to notify the parent when the child is ready.
    //
    SOCKET fds[2];
    IceInternal::createPipe(fds);

    //
    // Fork the child.
    //
    pid_t pid = fork();
    if(pid < 0)
    {
        cerr << argv[0] << ": " << strerror(errno) << endl;
        status = EXIT_FAILURE;
        return true;
    }

    if(pid != 0)
    {
        //
        // Parent process.
        //

        //
        // Close an unused end of the pipe.
        //
        close(fds[1]);

        //
        // Wait for the child to write a byte to the pipe to indicate that it
        // is ready to receive requests, or that an error occurred.
        //
        char c = 0;
        while(true)
        {
            if(read(fds[0], &c, 1) == -1)
            {
                if(IceInternal::interrupted())
                {
                    continue;
                }

                cerr << argv[0] << ": " << strerror(errno) << endl;
                status = EXIT_FAILURE;
                return true;
            }
            break;
        }

        if(c != 0)
        {
            //
            // Read an error message.
            //
            char msg[1024];
            size_t pos = 0;
            while(pos < sizeof(msg))
            {
                int n = read(fds[0], &msg[pos], sizeof(msg) - pos);
                if(n == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        continue;
                    }

                    cerr << argv[0] << ": I/O error while reading error message from child:" << endl
                         << strerror(errno) << endl;
                    status = EXIT_FAILURE;
                    return true;
                }
                pos += n;
                break;
            }
            cerr << argv[0] << ": failure occurred in daemon:" << endl << msg << endl;
            status = EXIT_FAILURE;
        }
        else
        {
            status = EXIT_SUCCESS;
        }

        return true;
    }

    //
    // Child process.
    //

    string errMsg;
    try
    {
        //
        // Become a session and process group leader.
        //
        if(setsid() == -1)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }

        //
        // Conventional wisdom recommends ignoring SIGHUP and forking again in order
        // to avoid the possibility of acquiring a controlling terminal. However,
        // doing this means the grandchild is no longer a process group leader, and
        // that would interfere with signal delivery on non-NPTL Linux systems.
        //
/*
        //
        // Ignore SIGHUP so that the grandchild process is not sent SIGHUP when this
        // process exits.
        //
        signal(SIGHUP, SIG_IGN);

        //
        // Fork again to eliminate the possibility of acquiring a controlling terminal.
        //
        pid = fork();
        if(pid < 0)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            throw ex;
        }
        if(pid != 0)
        {
            exit(0);
        }
*/

        //
        // Set the umask.
        //
        umask(0);

        if(!noChdir)
        {
            //
            // Change the working directory.
            //
            if(chdir("/") != 0)
            {
                SyscallException ex(__FILE__, __LINE__);
                ex.error = getSystemErrno();
                throw ex;
            }
        }

        fd_set fdsToClose;
        int fdMax;
        if(!noClose)
        {
            //
            // Take a snapshot of the open file descriptors. We don't actually close these
            // descriptors until after the communicator is initialized, so that plug-ins
            // have an opportunity to use stdin/stdout/stderr if necessary. This also
            // conveniently allows the Ice.PrintProcessId property to work as expected.
            //
            FD_ZERO(&fdsToClose);
            fdMax = sysconf(_SC_OPEN_MAX);
            if(fdMax <= 0)
            {
                SyscallException ex(__FILE__, __LINE__);
                ex.error = getSystemErrno();
                throw ex;
            }

            for(int i = 0; i < fdMax; ++i)
            {
                if(fcntl(i, F_GETFL) != -1)
                {
                    FD_SET(i, &fdsToClose);
                }
            }
            FD_CLR(fds[1], &fdsToClose); // Don't close the write end of the pipe.
        }

        //
        // Create the CtrlCHandler after forking the child so that signals are initialized
        // properly. We do this before initializing the communicator because we need to
        // ensure that signals are initialized before additional threads are created. The
        // communicator thread pools currently use lazy initialization, but a thread can
        // be created if Ice.MonitorConnections is defined.
        //
        _ctrlCHandler = new IceUtil::CtrlCHandler;

        //
        // Initialize the communicator.
        //
        _communicator = initializeCommunicator(argc, argv);

        if(!noClose)
        {
            //
            // Close unnecessary file descriptors.
            //
            for(int i = 0; i < fdMax; ++i)
            {
                if(FD_ISSET(i, &fdsToClose))
                {
                    close(i);
                }
            }

            //
            // Associate stdin, stdout and stderr with /dev/null.
            //
            int fd;
            fd = open("/dev/null", O_RDWR);
            assert(fd == 0);
            fd = dup2(0, 1);
            assert(fd == 1);
            fd = dup2(1, 2);
            assert(fd == 2);
        }

        //
        // Use the configured logger.
        //
        _logger = _communicator->getLogger();

        //
        // Start the service.
        //
        if(start(argc, argv))
        {
            //
            // Notify the parent that the child is ready.
            //
            char c = 0;
            while(true)
            {
                if(write(fds[1], &c, 1) == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        continue;
                    }
                }
                break;
            }
            close(fds[1]);
            fds[1] = 0;

            //
            // Wait for service shutdown.
            //
            waitForShutdown();

            //
            // Stop the service.
            //
            if(stop())
            {
                status = EXIT_SUCCESS;
            }
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        ostringstream ostr;
        ostr << "service caught unhandled Ice exception:" << endl << ex;
        errMsg = ostr.str();
        error(errMsg);
    }
    catch(...)
    {
        errMsg = "service caught unhandled C++ exception";
        error(errMsg);
    }

    //
    // If the service failed and the pipe to the parent is still open,
    // then send an error notification to the parent.
    //
    if(status != EXIT_SUCCESS && fds[1] != 0)
    {
        char c = 1;
        while(true)
        {
            if(write(fds[1], &c, 1) == -1)
            {
                if(IceInternal::interrupted())
                {
                    continue;
                }
            }
            break;
        }
        const char* msg = errMsg.c_str();
        size_t len = strlen(msg) + 1; // Include null byte
        size_t pos = 0;
        while(len > 0)
        {
            int n = write(fds[1], &msg[pos], len);
            if(n == -1)
            {
                if(IceInternal::interrupted())
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            len -= n;
            pos += n;
        }
        close(fds[1]);
    }

    try
    {
        _communicator->destroy();
    }
    catch(...)
    {
    }

    return true;
}

#endif
