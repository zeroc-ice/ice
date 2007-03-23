// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/CtrlCHandler.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/ArgVector.h>
#include <Ice/Service.h>
#include <Ice/LoggerI.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Properties.h>

#ifdef _WIN32
#   include <winsock2.h>
#   include <Ice/EventLoggerMsg.h>
#else
#   include <Ice/Logger.h>
#   include <Ice/Network.h>
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <csignal>
#   include <fstream>
#endif

using namespace std;
using namespace Ice;

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

//
// Main function for Win32 service.
//
void WINAPI
Ice_Service_ServiceMain(DWORD argc, LPTSTR* argv)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != 0);
    service->serviceMain(argc, argv);
}

//
// Win32 service control handler.
//
void WINAPI
Ice_Service_CtrlHandler(DWORD ctrl)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != 0);
    service->control(ctrl);
}

namespace
{

class ServiceStatusManager : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ServiceStatusManager(SERVICE_STATUS_HANDLE);

    //
    // Start a thread to provide regular status updates to the SCM.
    //
    void startUpdate(DWORD);

    //
    // Stop the update thread.
    //
    void stopUpdate();

    //
    // Change the service status and report it (once).
    //
    void changeStatus(DWORD, DWORD);

    //
    // Report the current status.
    //
    void reportStatus();

private:

    void run();

    class StatusThread : public IceUtil::Thread
    {
    public:

        StatusThread(ServiceStatusManager* manager) :
            _manager(manager)
        {
        }

        virtual void run()
        {
            _manager->run();
        }

    private:

        ServiceStatusManager* _manager;
    };
    friend class StatusThread;

    SERVICE_STATUS_HANDLE _handle;
    SERVICE_STATUS _status;
    IceUtil::ThreadPtr _thread;
    bool _stopped;
};

static ServiceStatusManager* serviceStatusManager;

static IceUtil::StaticMutex outputMutex = ICE_STATIC_MUTEX_INITIALIZER;

class SMEventLoggerI : public Ice::Logger
{
public:

    SMEventLoggerI(const string& service)
    {
        _source = RegisterEventSource(0, mangleService(service).c_str());
        if(_source == 0)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = GetLastError();
            throw ex;
        }
    }

    ~SMEventLoggerI()
    {
        assert(_source != 0);
        DeregisterEventSource(_source);
    }
    
    static void
    addKeys(const string& service)
    {
        HKEY hKey;
        DWORD d;
        LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE, createKey(service).c_str(), 0, "REG_SZ",
                                  REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, &d);
        if(err != ERROR_SUCCESS)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = err;
            throw ex;
        }

        //
        // Get the filename of this DLL.
        //
        char path[_MAX_PATH];
        assert(_module != 0);
        if(!GetModuleFileName(_module, path, _MAX_PATH))
        {
            RegCloseKey(hKey);
            SyscallException ex(__FILE__, __LINE__);
            ex.error = GetLastError();
            throw ex;
        }

        //
        // The event resources are bundled into this DLL, therefore
        // the "EventMessageFile" key should contain the path to this
        // DLL.
        //
        err = RegSetValueEx(hKey, "EventMessageFile", 0, REG_EXPAND_SZ, 
                            reinterpret_cast<unsigned char*>(path), static_cast<DWORD>(strlen(path) + 1));
        if(err == ERROR_SUCCESS)
        {
            //
            // The "TypesSupported" key indicates the supported event
            // types.
            //
            DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            err = RegSetValueEx(hKey, "TypesSupported", 0, REG_DWORD,
                                reinterpret_cast<unsigned char*>(&typesSupported), sizeof(typesSupported));
        }
        if(err != ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            SyscallException ex(__FILE__, __LINE__);
            ex.error = err;
            throw ex;
        }

        RegCloseKey(hKey);
    }

    static void
    removeKeys(const string& theService)
    {
        LONG err = RegDeleteKey(HKEY_LOCAL_MACHINE, createKey(theService).c_str());
        if(err != ERROR_SUCCESS)
        {
            SyscallException ex(__FILE__, __LINE__);
            ex.error = err;
            throw ex;
        }
    }

    virtual void
    print(const string& message)
    {
        const char* str[1];
        str[0] = message.c_str();
        //
        // We ignore any failures from ReportEvent since there isn't
        // anything we can do about it.
        //
        ReportEvent(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0);
    }

    void
    trace(const string& category, const string& message)
    {
        string s;
        if(!category.empty())
        {
            s = category;
            s.append(": ");
        }
        s.append(message);

        const char* str[1];
        str[0] = s.c_str();
        //
        // We ignore any failures from ReportEvent since there isn't
        // anything we can do about it.
        //
        ReportEvent(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0);
    }

    virtual void
    warning(const string& message)
    {
        const char* str[1];
        str[0] = message.c_str();
        //
        // We ignore any failures from ReportEvent since there isn't
        // anything we can do about it.
        //
        ReportEvent(_source, EVENTLOG_WARNING_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0);
    }

    virtual void
    error(const string& message)
    {
        const char* str[1];
        str[0] = message.c_str();
        //
        // We ignore any failures from ReportEvent since there isn't
        // anything we can do about it.
        //
        ReportEvent(_source, EVENTLOG_ERROR_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, str, 0);
    }

    static void
    setModuleHandle(HMODULE module)
    {
        _module = module;
    }
    
private:

    static string
    mangleService(string name)
    {
        //
        // The service name cannot contain backslashes.
        //
        string::size_type pos = 0;
        while((pos = name.find('\\', pos)) != string::npos)
        {
            name[pos] = '/';
        }
        return name;
    }

    static string
    createKey(string name)
    {
        //
        // The registry key is:
        //
        // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application.
        //
        return "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + mangleService(name);
    }


    HANDLE _source;
    static HMODULE _module;
};

HMODULE SMEventLoggerI::_module = 0;

}

#endif

Ice::Service::Service()
{
    assert(_instance == 0);
    _nohup = true;
    _service = false;
    _instance = this;

#ifndef _WIN32
    _changeDirectory = true;
    _closeFiles = true;
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
        try
        {
            _communicator->shutdown();
        }
        catch(const CommunicatorDestroyedException&)
        {
            //
            // Expected if the service communicator is being destroyed.
            //
        }
        catch(const Ice::Exception& ex)
        {
            ostringstream ostr;
            ostr << "exception during shutdown:\n" << ex;
            warning(ostr.str());
        }
        catch(...)
        {
            warning("unknown exception during shutdown");
        }
    }
    return true;
}

void
Ice::Service::interrupt()
{
    shutdown();
}

int
Ice::Service::main(int& argc, char* argv[], const InitializationData& initData)
{
    _name = argv[0];

#ifdef _WIN32
    //
    // First check for the --service option.
    //
    string name;
    int idx = 1;
    while(idx < argc)
    {
        if(strcmp(argv[idx], "--service") == 0)
        {
            if(idx + 1 >= argc)
            {
                error("service name argument expected for `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            name = argv[idx + 1];

            //
            // If the process logger the default logger then we use
            // our own logger.
            //
            _logger = getProcessLogger();
            if(LoggerIPtr::dynamicCast(_logger))
            {
                _logger = new SMEventLoggerI(name);
                setProcessLogger(_logger);
            }

            for(int i = idx; i + 2 < argc; ++i)
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
            if(!op.empty())
            {
                error("cannot specify `" + op + "' and `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            if(!name.empty())
            {
                error("cannot specify `--service' and `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            if(idx + 1 >= argc)
            {
                error("service name argument expected for `" + string(argv[idx]) + "'");
                return EXIT_FAILURE;
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
            //
            // Check for --display, --executable.
            //
            string display, executable;
            idx = 1;
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

            vector<string> args;
            //
            // Prepend the arguments "--service NAME" so that the service
            // starts properly.
            //
            args.push_back("--service");
            args.push_back(name);
            for(idx = 1; idx < argc; ++idx)
            {
                args.push_back(argv[idx]);
            }
            try
            {
                //
                // Add the registry keys for the event logger if
                // initData.logger is empty (which is the case if the
                // user wants to use the service default logger).
                //
                return installService(!initData.logger, name, display, executable, args);
            }
            catch(const Ice::Exception& ex)
            {
                ostringstream ostr;
                ostr << ex;
                error(ostr.str());
                return EXIT_FAILURE;
            }
        }
        else if(op == "--uninstall")
        {
            try
            {
                return uninstallService(!initData.logger, name);
            }
            catch(const Ice::Exception& ex)
            {
                ostringstream ostr;
                ostr << ex;
                error(ostr.str());
                return EXIT_FAILURE;
            }
        }
        else if(op == "--start")
        {
            vector<string> args;
            for(idx = 1; idx < argc; ++idx)
            {
                args.push_back(argv[idx]);
            }
            return startService(name, args);
        }
        else
        {
            assert(op == "--stop");
            return stopService(name);
        }
    }

    if(!name.empty())
    {
        configureService(name);
    }
#else
    //
    // Check for --daemon, --noclose, --nochdir and --pidfile.
    //
    bool daemonize = false;
    bool closeFiles = true;
    bool changeDirectory = true;
    string pidFile;
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

            closeFiles = false;
        }
        else if(strcmp(argv[idx], "--nochdir") == 0)
        {
            for(int i = idx; i + 1 < argc; ++i)
            {
                argv[i] = argv[i + 1];
            }
            argc -= 1;

            changeDirectory = false;
        }
        else if(strcmp(argv[idx], "--pidfile") == 0)
        {
            if(idx + 1 < argc)
            {
                pidFile = argv[idx + 1];
            }
            else
            {
                cerr << argv[0] << ": --pidfile must be followed by an argument" << endl;
                return EXIT_FAILURE;
            }

            for(int i = idx; i + 2 < argc; ++i)
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

    if(!closeFiles && !daemonize)
    {
        cerr << argv[0] << ": --noclose must be used with --daemon" << endl;
        return EXIT_FAILURE;
    }

    if(pidFile.size() > 0 && !daemonize)
    {
        cerr << argv[0] << ": --pidfile <file> must be used with --daemon" << endl;
        return EXIT_FAILURE;
    }

    if(daemonize)
    {
        configureDaemon(changeDirectory, closeFiles, pidFile);
    }
#endif

    return run(argc, argv, initData);
}

int
Ice::Service::main(StringSeq& args, const InitializationData& initData)
{
    IceUtil::ArgVector av(args);
    int rc = main(av.argc, av.argv, initData);
    argsToStringSeq(av.argc, av.argv);
    return rc;
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

bool
Ice::Service::service() const
{
    return _service;
}

string
Ice::Service::name() const
{
    return _name;
}

bool
Ice::Service::checkSystem() const
{
#ifdef _WIN32
    //
    // Check Windows version.
    //
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);
    return (ver.dwPlatformId == VER_PLATFORM_WIN32_NT);
#else
    return true;
#endif
}

int
Ice::Service::run(int& argc, char* argv[], const InitializationData& initData)
{
    if(_service)
    {
#ifdef _WIN32
        return runService(argc, argv, initData);
#else
        return runDaemon(argc, argv, initData);
#endif
    }

    //
    // Run as a foreground process.
    //
    int status = EXIT_FAILURE;
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
        _communicator = initializeCommunicator(argc, argv, initData);

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
        ostr << "service caught unhandled Ice exception:\n" << ex;
        error(ostr.str());
    }
    catch(const std::exception& ex)
    {
        ostringstream ostr;
        ostr << "service caught unhandled std::exception:\n" << ex.what();
        error(ostr.str());
    }
    catch(const std::string& msg)
    {
        ostringstream ostr;
        ostr << "service caught unhandled exception:\n" << msg;
        error(ostr.str());
    }
    catch(const char* msg)
    {
        ostringstream ostr;
        ostr << "service caught unhandled exception:\n" << msg;
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

#ifdef _WIN32

void
Ice::Service::configureService(const string& name)
{
    _service = true;
    _name = name;
}

int
Ice::Service::installService(bool useEventLogger, const string& name, const string& display, const string& executable,
                             const vector<string>& args)
{
    string disp, exec;

    disp = display;
    if(disp.empty())
    {
        disp = name;
    }

    exec = executable;
    if(exec.empty())
    {
        //
        // Use this executable if none is specified.
        //
        char buf[_MAX_PATH];
        if(GetModuleFileName(0, buf, _MAX_PATH) == 0)
        {
            error("unable to obtain file name of executable");
            return EXIT_FAILURE;
        }
        exec = buf;
    }

    //
    // Compose service command line. The executable and any arguments must
    // be enclosed in quotes if they contain whitespace.
    //
    string command;
    if(executable.find(' ') != string::npos)
    {
        command.push_back('"');
        command.append(exec);
        command.push_back('"');
    }
    else
    {
        command = exec;
    }
    for(vector<string>::const_iterator p = args.begin(); p != args.end(); ++p)
    {
        command.push_back(' ');

        if(p->find_first_of(" \t\n\r") != string::npos)
        {
            command.push_back('"');
            command.append(*p);
            command.push_back('"');
        }
        else
        {
            command.append(*p);
        }
    }

    SC_HANDLE hSCM = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(hSCM == 0)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }
    SC_HANDLE hService = CreateService(
        hSCM,
        name.c_str(),
        disp.c_str(),
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        command.c_str(),
        0,
        0,
        0,
        0,
        0);

    if(hService == 0)
    {
        syserror("unable to install service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hSCM);
    CloseServiceHandle(hService);

    //
    // Add the registry keys for the event logger if _logger is
    // empty (which is the case if the user wants to use
    // the service default logger).
    //
    if(useEventLogger)
    {
        SMEventLoggerI::addKeys(name);
    }

    return EXIT_SUCCESS;
}

int
Ice::Service::uninstallService(bool useEventLogger, const string& name)
{
    SC_HANDLE hSCM = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(hSCM == 0)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SERVICE_ALL_ACCESS);
    if(hService == 0)
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

    //
    // Remove the registry keys for the event logger if necessary.
    //
    if(useEventLogger)
    {
        SMEventLoggerI::removeKeys(name);
    }

    return EXIT_SUCCESS;
}

int
Ice::Service::startService(const string& name, const vector<string>& args)
{
    SC_HANDLE hSCM = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(hSCM == 0)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SERVICE_ALL_ACCESS);
    if(hService == 0)
    {
        syserror("unable to open service `" + name + "'");
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    //
    // Create argument vector. Note that StartService() automatically
    // adds the service name in argv[0], so the argv that is passed to
    // StartService() must *not* include the the service name in
    // argv[0].
    //
    const int argc = static_cast<int>(args.size());
    LPCSTR* argv = new LPCSTR[argc];
    int i = 0;
    for(vector<string>::const_iterator p = args.begin(); p != args.end(); ++p)
    {
        argv[i++] = strdup(p->c_str());
    }

    //
    // Start service.
    //
    BOOL b = StartService(hService, argc, argv);

    //
    // Clean up argument vector.
    //
    for(i = 0; i < argc; ++i)
    {
        free(const_cast<char*>(argv[i]));
    }
    delete[] argv;

    if(!b)
    {
        syserror("unable to start service `" + name + "'");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    trace("Service start pending.");

    //
    // Wait until the service is started or an error is detected.
    //
    SERVICE_STATUS status;
    if(!waitForServiceState(hService, SERVICE_START_PENDING, status))
    {
        syserror("unable to query status of service `" + name + "'");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    if(status.dwCurrentState == SERVICE_RUNNING)
    {
        trace("Service is running.");
    }
    else
    {
        showServiceStatus("Service failed to start.", status);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
Ice::Service::stopService(const string& name)
{
    SC_HANDLE hSCM = OpenSCManager(0, 0, SC_MANAGER_ALL_ACCESS);
    if(hSCM == 0)
    {
        syserror("failure in OpenSCManager");
        return EXIT_FAILURE;
    }

    SC_HANDLE hService = OpenService(hSCM, name.c_str(), SERVICE_ALL_ACCESS);
    if(hService == 0)
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

    trace("Service stop pending.");

    //
    // Wait until the service is stopped or an error is detected.
    //
    if(!waitForServiceState(hService, SERVICE_STOP_PENDING, status))
    {
        syserror("unable to query status of service `" + name + "'");
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCM);
        return EXIT_FAILURE;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCM);

    if(status.dwCurrentState == SERVICE_STOPPED)
    {
        trace("Service is stopped.");
    }
    else
    {
        showServiceStatus("Service failed to stop.", status);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
Ice::Service::setModuleHandle(HMODULE module)
{
    SMEventLoggerI::setModuleHandle(module);
}

#else

void
Ice::Service::configureDaemon(bool changeDirectory, bool closeFiles, const string& pidFile)
{
    _service = true;
    _changeDirectory = changeDirectory;
    _closeFiles = closeFiles;
    _pidFile = pidFile;
}

#endif

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
Ice::Service::initializeCommunicator(int& argc, char* argv[], const InitializationData& initData)
{
    return Ice::initialize(argc, argv, initData);
}

void
Ice::Service::syserror(const string& msg)
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
                                 0,
                                 err,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                 (LPTSTR)&lpMsgBuf,
                                 0,
                                 0);
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
        cerr << _name << ": ";
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
Ice::Service::error(const string& msg)
{
    if(_logger)
    {
        _logger->error(msg);
    }
    else
    {
        cerr << _name << ": error: " << msg << endl;
    }
}

void
Ice::Service::warning(const string& msg)
{
    if(_logger)
    {
        _logger->warning(msg);
    }
    else
    {
        cerr << _name << ": warning: " << msg << endl;
    }
}

void
Ice::Service::trace(const string& msg)
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

void
Ice::Service::print(const string& msg)
{
    if(_logger)
    {
        _logger->print(msg);
    }
    else
    {
        cerr << msg << endl;
    }
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

#ifdef _WIN32

int
Ice::Service::runService(int argc, char* argv[], const InitializationData& initData)
{
    assert(_service);

    if(!checkSystem())
    {
        error("Win32 service not supported on Windows 9x/ME");
        return EXIT_FAILURE;
    }

    if(_name.empty())
    {
        error("invalid name for Win32 service");
        return EXIT_FAILURE;
    }

    //
    // Arguments passed to the executable are not passed to the service's main function,
    // so save them now and serviceMain will merge them later.
    //
    for(int idx = 1; idx < argc; ++idx)
    {
        _serviceArgs.push_back(argv[idx]);
    }

    _initData = initData;

    SERVICE_TABLE_ENTRY ste[] =
    {
        { const_cast<char*>(_name.c_str()), Ice_Service_ServiceMain },
        { 0, 0 },
    };

    //
    // Start the service.
    //
    if(!StartServiceCtrlDispatcher(ste))
    {
        syserror("unable to start service control dispatcher");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void
Ice::Service::terminateService(DWORD exitCode)
{
    serviceStatusManager->stopUpdate();
    delete serviceStatusManager;
    serviceStatusManager = 0;

    SERVICE_STATUS status;

    status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    status.dwCurrentState = SERVICE_STOPPED;
    status.dwControlsAccepted = 0;
    if(exitCode != 0)
    {
        status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
    }
    else
    {
        status.dwWin32ExitCode = 0;
    }
    status.dwServiceSpecificExitCode = exitCode;
    status.dwCheckPoint = 0;
    status.dwWaitHint = 0;

    SetServiceStatus(_statusHandle, &status);
}

bool
Ice::Service::waitForServiceState(SC_HANDLE hService, DWORD pendingState, SERVICE_STATUS& status)
{
    if(!QueryServiceStatus(hService, &status))
    {
        return false;
    }

    //
    // Save the tick count and initial checkpoint.
    //
    DWORD startTickCount = GetTickCount();
    DWORD oldCheckPoint = status.dwCheckPoint;

    //
    // Loop while the service is in the pending state.
    //
    while(status.dwCurrentState == pendingState)
    {
        //
        // Do not wait longer than the wait hint. A good interval is
        // one tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.
        //

        DWORD waitTime = status.dwWaitHint / 10;

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
            return false;
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

    return true;
}

void
Ice::Service::showServiceStatus(const string& msg, SERVICE_STATUS& status)
{
    string state;
    switch(status.dwCurrentState)
    {
    case SERVICE_STOPPED:
        state = "STOPPED";
        break;
    case SERVICE_START_PENDING:
        state = "START PENDING";
        break;
    case SERVICE_STOP_PENDING:
        state = "STOP PENDING";
        break;
    case SERVICE_RUNNING:
        state = "RUNNING";
        break;
    case SERVICE_CONTINUE_PENDING:
        state = "CONTINUE PENDING";
        break;
    case SERVICE_PAUSE_PENDING:
        state = "PAUSE PENDING";
        break;
    case SERVICE_PAUSED:
        state = "PAUSED";
        break;
    default:
        state = "UNKNOWN";
        break;
    }

    ostringstream ostr;
    ostr << msg << endl
         << "  Current state: " << state << endl
         << "  Exit code: " << status.dwWin32ExitCode << endl
         << "  Service specific exit code: " << status.dwServiceSpecificExitCode << endl
         << "  Check point: " << status.dwCheckPoint << endl
         << "  Wait hint: " << status.dwWaitHint;
    trace(ostr.str());
}

void
Ice::Service::serviceMain(int argc, char* argv[])
{
    _ctrlCHandler = new IceUtil::CtrlCHandler;

    //
    // Register the control handler function.
    //
    _statusHandle = RegisterServiceCtrlHandler(argv[0], Ice_Service_CtrlHandler);
    if(_statusHandle == (SERVICE_STATUS_HANDLE)0)
    {
        syserror("unable to register service control handler");
        return;
    }

    //
    // Create the service status manager and start a thread to periodically
    // update the service's status with the service control manager (SCM).
    // The SCM must receive periodic updates otherwise it assumes that
    // initialization failed and terminates the service.
    //
    serviceStatusManager = new ServiceStatusManager(_statusHandle);
    serviceStatusManager->startUpdate(SERVICE_START_PENDING);

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
    argc += static_cast<int>(_serviceArgs.size());

    //
    // If we can't initialize a communicator, then stop immediately.
    //
    try
    {
        _communicator = initializeCommunicator(argc, args, _initData);
    }
    catch(const Ice::Exception& ex)
    {
        delete[] args;
        ostringstream ostr;
        ostr << "exception occurred while initializing a communicator:\n" << ex;
        error(ostr.str());
        terminateService(EXIT_FAILURE);
        return;
    }
    catch(...)
    {
        delete[] args;
        error("unknown exception occurred while initializing a communicator");
        terminateService(EXIT_FAILURE);
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
            // Change the current status from START_PENDING to RUNNING.
            //
            serviceStatusManager->stopUpdate();
            serviceStatusManager->changeStatus(SERVICE_RUNNING, SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);

            //
            // Wait for the service to be shut down.
            //
            waitForShutdown();

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
        ostr << "service caught unhandled Ice exception:\n" << ex;
        error(ostr.str());
    }
    catch(...)
    {
        error("service caught unhandled C++ exception");
    }

    delete[] args;

    try
    {
        _communicator->destroy();
    }
    catch(...)
    {
    }

    terminateService(status);
}

void
Ice::Service::control(int ctrl)
{
    assert(serviceStatusManager);

    switch(ctrl)
    {
    case SERVICE_CONTROL_SHUTDOWN:
    case SERVICE_CONTROL_STOP:
    {
        serviceStatusManager->startUpdate(SERVICE_STOP_PENDING);
        shutdown();
        break;
    }
    default:
    {
        if(ctrl != SERVICE_CONTROL_INTERROGATE)
        {
            ostringstream ostr;
            ostr << "unrecognized service control code " << ctrl;
            error(ostr.str());
        }

        serviceStatusManager->reportStatus();
        break;
    }
    }
}

ServiceStatusManager::ServiceStatusManager(SERVICE_STATUS_HANDLE handle) :
    _handle(handle), _stopped(false)
{
    _status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    _status.dwControlsAccepted = 0;
    _status.dwWin32ExitCode = 0;
    _status.dwServiceSpecificExitCode = 0;
    _status.dwCheckPoint = 0;
    _status.dwWaitHint = 0;
}

void
ServiceStatusManager::startUpdate(DWORD state)
{
    Lock sync(*this);

    assert(state == SERVICE_START_PENDING || state == SERVICE_STOP_PENDING);
    assert(!_thread);

    _status.dwCurrentState = state;
    _status.dwControlsAccepted = 0; // Don't accept any other control messages while pending.

    _stopped = false;

    _thread = new StatusThread(this);
    _thread->start();
}

void
ServiceStatusManager::stopUpdate()
{
    IceUtil::ThreadPtr thread;

    {
        Lock sync(*this);

        if(_thread)
        {
            _stopped = true;
            notify();
            thread = _thread;
            _thread = 0;
        }
    }

    if(thread)
    {
        thread->getThreadControl().join();
    }
}

void
ServiceStatusManager::changeStatus(DWORD state, DWORD controlsAccepted)
{
    Lock sync(*this);

    _status.dwCurrentState = state;
    _status.dwControlsAccepted = controlsAccepted;

    SetServiceStatus(_handle, &_status);
}

void
ServiceStatusManager::reportStatus()
{
    Lock sync(*this);

    SetServiceStatus(_handle, &_status);
}

void
ServiceStatusManager::run()
{
    Lock sync(*this);

    IceUtil::Time delay = IceUtil::Time::milliSeconds(1000);
    _status.dwWaitHint = 2000;
    _status.dwCheckPoint = 0;

    while(!_stopped)
    {
        _status.dwCheckPoint++;
        SetServiceStatus(_handle, &_status);
        timedWait(delay);
    }
}

#else

int
Ice::Service::runDaemon(int argc, char* argv[], const InitializationData& initData)
{
    assert(_service);

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
        return EXIT_FAILURE;
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
                _exit(EXIT_FAILURE);
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
                ssize_t n = read(fds[0], &msg[pos], sizeof(msg) - pos);
                if(n == -1)
                {
                    if(IceInternal::interrupted())
                    {
                        continue;
                    }

                    cerr << argv[0] << ": I/O error while reading error message from child:\n"
                         << strerror(errno) << endl;
                    _exit(EXIT_FAILURE);
                }
                pos += n;
                break;
            }
            cerr << argv[0] << ": failure occurred in daemon";
            if(strlen(msg) > 0)
            {
                cerr << ':' << endl << msg;
            }
            cerr << endl;
            _exit(EXIT_FAILURE);
        }

        _exit(EXIT_SUCCESS);
    }

    //
    // Child process.
    //

    string errMsg;
    int status = EXIT_FAILURE;
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

        if(_changeDirectory)
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

        vector<int> fdsToClose;
        if(_closeFiles)
        {
            //
            // Take a snapshot of the open file descriptors. We don't actually close these
            // descriptors until after the communicator is initialized, so that plug-ins
            // have an opportunity to use stdin/stdout/stderr if necessary. This also
            // conveniently allows the Ice.PrintProcessId property to work as expected.
            //
            int fdMax = static_cast<int>(sysconf(_SC_OPEN_MAX));
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
                    //
                    // Don't close the write end of the pipe.
                    //
                    if(i != fds[1])
                    {
                       fdsToClose.push_back(i);
                    }
                }
            }
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
        _communicator = initializeCommunicator(argc, argv, initData);

        if(_closeFiles)
        {
            //
            // Close unnecessary file descriptors.
            //
            PropertiesPtr properties = _communicator->getProperties();
            string stdOut = properties->getProperty("Ice.StdOut");
            string stdErr = properties->getProperty("Ice.StdErr");

            vector<int>::const_iterator p;
            for(p = fdsToClose.begin(); p != fdsToClose.end(); ++p)
            {
                //
                // NOTE: Do not close stdout if Ice.StdOut is defined. Likewise for Ice.StdErr.
                //
                if((*p == 1 && !stdOut.empty()) || (*p == 2 && !stdErr.empty()))
                {
                    continue;
                }
                close(*p);
            }

            //
            // Associate stdin, stdout and stderr with /dev/null.
            //
            int fd;
            fd = open("/dev/null", O_RDWR);
            assert(fd == 0);
            if(stdOut.empty())
            {
                fd = dup2(0, 1);
                assert(fd == 1);
            }
            if(stdErr.empty())
            {
                fd = dup2(1, 2);
                assert(fd == 2);
            }
        }
        
        //
        // Write PID
        //
        if(_pidFile.size() > 0)
        {
            ofstream of(_pidFile.c_str());
            of << getpid() << endl;

            if(!of)
            {
                warning("Could not write PID file " + _pidFile);
            }
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
            fds[1] = -1;

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
        ostr << "service caught unhandled Ice exception:\n" << ex;
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
    if(status != EXIT_SUCCESS && fds[1] != -1)
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
            ssize_t n = write(fds[1], &msg[pos], len);
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

    return status;
}

#endif
