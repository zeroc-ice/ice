// Copyright (c) ZeroC, Inc.

#include "Ice/Service.h"
#include "ArgVector.h"
#include "ConsoleUtil.h"
#include "Ice/StringUtil.h"
#include "LoggerI.h"

#ifdef _WIN32
#    include "Ice/EventLoggerMsg.h"
#    include <winsock2.h>
#else
#    include "Network.h"
#    include <csignal>
#    include <sys/stat.h>
#    include <sys/types.h>
#    ifdef ICE_USE_SYSTEMD
#        include <systemd/sd-daemon.h>
#    endif
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

Ice::Service* Ice::Service::_instance = nullptr;

namespace
{
    CtrlCHandler* ctrlCHandler = nullptr;
}

//
// Callback for CtrlCHandler.
//
static void
ctrlCHandlerCallback(int sig)
{
    Ice::Service* service = Ice::Service::instance();
    assert(service != nullptr);
    service->handleInterrupt(sig);
}

#ifdef _WIN32

//
// Main function for Win32 service.
//
void WINAPI
Ice_Service_ServiceMain(DWORD argc, LPWSTR* argv)
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
    class ServiceStatusManager
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

        SERVICE_STATUS_HANDLE _handle;
        SERVICE_STATUS _status;
        std::thread _thread;
        bool _stopped;
        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };

    static ServiceStatusManager* serviceStatusManager;

    //
    // Interface implemented by SMEventLoggerI and called from
    // SMEventLoggerIWrapper.
    //
    class SMEventLogger
    {
    public:
        virtual void print(const string&, const string&) = 0;
        virtual void trace(const string&, const string&, const string&) = 0;
        virtual void warning(const string&, const string&) = 0;
        virtual void error(const string&, const string&) = 0;
    };
    using SMEventLoggerPtr = std::shared_ptr<SMEventLogger>;

    class SMEventLoggerIWrapper final : public Ice::Logger
    {
    public:
        SMEventLoggerIWrapper(const SMEventLoggerPtr& logger, string prefix)
            : _logger(logger),
              _prefix(std::move(prefix))
        {
            assert(_logger);
        }

        void print(const string& message) final { _logger->print(_prefix, message); }

        void trace(const string& category, const string& message) final { _logger->trace(_prefix, category, message); }

        void warning(const string& message) final { _logger->warning(_prefix, message); }

        void error(const string& message) final { _logger->error(_prefix, message); }

        string getPrefix() final { return _prefix; }

        Ice::LoggerPtr cloneWithPrefix(string prefix) final
        {
            return make_shared<SMEventLoggerIWrapper>(_logger, std::move(prefix));
        }

    private:
        SMEventLoggerPtr _logger;
        const string _prefix;
    };

    class SMEventLoggerI : public SMEventLogger
    {
    public:
        SMEventLoggerI(const string& source, const StringConverterPtr& stringConverter)
            : _stringConverter(stringConverter)
        {
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            _source = RegisterEventSourceW(0, stringToWstring(mangleSource(source), _stringConverter).c_str());
            if (_source == 0)
            {
                throw SyscallException{__FILE__, __LINE__, "RegisterEventSourceW failed", GetLastError()};
            }
        }

        ~SMEventLoggerI()
        {
            assert(_source != 0);
            DeregisterEventSource(_source);
        }

        static void addKeys(const string& source, const StringConverterPtr& stringConverter)
        {
            HKEY hKey;
            DWORD d;
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            LSTATUS err = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                stringToWstring(createKey(source), stringConverter).c_str(),
                0,
                const_cast<wchar_t*>(L"REG_SZ"),
                REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS,
                0,
                &hKey,
                &d);

            if (err != ERROR_SUCCESS)
            {
                throw SyscallException{__FILE__, __LINE__, "RegCreateKeyExW failed", static_cast<DWORD>(err)};
            }

            //
            // Get the filename of this DLL.
            //
            wchar_t path[_MAX_PATH];
            assert(_module != 0);
            if (!GetModuleFileNameW(_module, path, _MAX_PATH))
            {
                DWORD error = GetLastError();
                RegCloseKey(hKey);
                throw SyscallException{__FILE__, __LINE__, "GetModuleFileNameW failed", error};
            }

            //
            // The event resources are bundled into this DLL, therefore
            // the "EventMessageFile" key should contain the path to this
            // DLL.
            //
            err = RegSetValueExW(
                hKey,
                L"EventMessageFile",
                0,
                REG_EXPAND_SZ,
                reinterpret_cast<unsigned char*>(path),
                static_cast<DWORD>((wcslen(path) * sizeof(wchar_t)) + 1));

            if (err == ERROR_SUCCESS)
            {
                //
                // The "TypesSupported" key indicates the supported event
                // types.
                //
                DWORD typesSupported = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
                err = RegSetValueExW(
                    hKey,
                    L"TypesSupported",
                    0,
                    REG_DWORD,
                    reinterpret_cast<unsigned char*>(&typesSupported),
                    sizeof(typesSupported));
            }
            if (err != ERROR_SUCCESS)
            {
                RegCloseKey(hKey);
                throw SyscallException{__FILE__, __LINE__, "RegSetValueExW failed", static_cast<DWORD>(err)};
            }

            RegCloseKey(hKey);
        }

        static void removeKeys(const string& source, const StringConverterPtr& stringConverter)
        {
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            LSTATUS err =
                RegDeleteKeyW(HKEY_LOCAL_MACHINE, stringToWstring(createKey(source), stringConverter).c_str());
            if (err != ERROR_SUCCESS)
            {
                throw SyscallException{__FILE__, __LINE__, "RegDeleteKeyW failed", static_cast<DWORD>(err)};
            }
        }

        virtual void print(const string& prefix, const string& message)
        {
            string s;
            if (!prefix.empty())
            {
                s = prefix;
                s.append(": ");
            }
            s.append(message);
            print(s);
        }

        void print(const string& message)
        {
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            const wstring msg = stringToWstring(message, _stringConverter);
            const wchar_t* messages[1];
            messages[0] = msg.c_str();
            //
            // We ignore any failures from ReportEvent since there isn't
            // anything we can do about it.
            //
            ReportEventW(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, messages, 0);
        }

        virtual void trace(const string& prefix, const string& category, const string& message)
        {
            string s;
            if (!category.empty())
            {
                s = category;
                s.append(": ");
            }
            s.append(message);
            trace(prefix, s);
        }

        void trace(const string& category, const string& message)
        {
            string s;
            if (!category.empty())
            {
                s = category;
                s.append(": ");
            }
            s.append(message);

            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            wstring msg = stringToWstring(s, _stringConverter);
            const wchar_t* messages[1];
            messages[0] = msg.c_str();
            //
            // We ignore any failures from ReportEvent since there isn't
            // anything we can do about it.
            //
            ReportEventW(_source, EVENTLOG_INFORMATION_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, messages, 0);
        }

        virtual void warning(const string& prefix, const string& message)
        {
            string s;
            if (!prefix.empty())
            {
                s = prefix;
                s.append(": ");
            }
            s.append(message);
            warning(s);
        }

        void warning(const string& message)
        {
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            wstring msg = stringToWstring(message, _stringConverter);
            const wchar_t* messages[1];
            messages[0] = msg.c_str();
            //
            // We ignore any failures from ReportEvent since there isn't
            // anything we can do about it.
            //
            ReportEventW(_source, EVENTLOG_WARNING_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, messages, 0);
        }

        virtual void error(const string& prefix, const string& message)
        {
            string s;
            if (!prefix.empty())
            {
                s = prefix;
                s.append(": ");
            }
            s.append(message);
            error(s);
        }

        void error(const string& message)
        {
            //
            // Don't need to use a wide string converter as the wide string is passed
            // to Windows API.
            //
            wstring msg = stringToWstring(message, _stringConverter);
            const wchar_t* messages[1];
            messages[0] = msg.c_str();
            //
            // We ignore any failures from ReportEvent since there isn't
            // anything we can do about it.
            //
            ReportEventW(_source, EVENTLOG_ERROR_TYPE, 0, EVENT_LOGGER_MSG, 0, 1, 0, messages, 0);
        }

        static void setModuleHandle(HMODULE module) { _module = module; }

    private:
        static string mangleSource(string name)
        {
            //
            // The source name cannot contain backslashes.
            //
            string::size_type pos = 0;
            while ((pos = name.find('\\', pos)) != string::npos)
            {
                name[pos] = '/';
            }
            return name;
        }

        static string createKey(string name)
        {
            //
            // The registry key is:
            //
            // HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\EventLog\Application.
            //
            return "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + mangleSource(name);
        }

        StringConverterPtr _stringConverter;
        HANDLE _source;
        static HMODULE _module;
    };

    HMODULE SMEventLoggerI::_module = 0;
}

#endif

Ice::Service::Service()
{
    assert(_instance == nullptr);
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
    _instance = nullptr;
    delete ctrlCHandler;
}

bool
Ice::Service::shutdown()
{
    if (_communicator)
    {
        try
        {
            _communicator->shutdown();
        }
        catch (const CommunicatorDestroyedException&)
        {
            //
            // Expected if the service communicator is being destroyed.
            //
        }
        catch (const Ice::Exception& ex)
        {
            ServiceWarning warn(this);
            warn << "exception during shutdown:\n" << ex;
        }
        catch (const std::exception& ex)
        {
            ServiceWarning warn(this);
            warn << "exception during shutdown:\n" << ex;
        }
        catch (...)
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
Ice::Service::main(int argc, const char* const argv[], InitializationData initData)
{
    _name = "";
    if (argc > 0)
    {
        _name = argv[0];
    }

    IceInternal::ArgVector av(argc, argv); // copy args

    //
    // We parse the properties here to extract Ice.ProgramName and
    // Ice.EventLog.Source on Windows.
    //
    try
    {
        initData.properties = createProperties(av.argc, av.argv, initData.properties);
    }
    catch (const Ice::Exception& ex)
    {
        ServiceError err(this);
        err << "createProperties failed: " << ex;
        return EXIT_FAILURE;
    }

#ifdef _WIN32

    //
    // First check for the --service option.
    //
    string name;
    int idx = 1;
    const StringConverterPtr stringConverter = getProcessStringConverter();
    while (idx < av.argc)
    {
        if (strcmp(av.argv[idx], "--service") == 0)
        {
            if (idx + 1 >= av.argc)
            {
                error("service name argument expected for '" + string(av.argv[idx]) + "'");
                return EXIT_FAILURE;
            }

            name = av.argv[idx + 1];

            //
            // If the process logger is the default logger then we use
            // our own logger.
            //
            _logger = getProcessLogger();
            if (dynamic_pointer_cast<LoggerI>(_logger))
            {
                string eventLogSource = initData.properties->getIceProperty("Ice.EventLog.Source");
                if (eventLogSource.empty())
                {
                    eventLogSource = name;
                }
                _logger = make_shared<SMEventLoggerIWrapper>(
                    make_shared<SMEventLoggerI>(eventLogSource, stringConverter),
                    "");
                setProcessLogger(_logger);
            }

            for (int i = idx; i + 2 < av.argc; ++i)
            {
                av.argv[i] = av.argv[i + 2];
            }
            av.argc -= 2;
        }
        else
        {
            ++idx;
        }
    }

    if (!name.empty())
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
    while (idx < av.argc)
    {
        if (strcmp(av.argv[idx], "--daemon") == 0)
        {
            for (int i = idx; i + 1 < av.argc; ++i)
            {
                av.argv[i] = av.argv[i + 1];
            }
            av.argc -= 1;

            daemonize = true;
        }
        else if (strcmp(av.argv[idx], "--noclose") == 0)
        {
            for (int i = idx; i + 1 < av.argc; ++i)
            {
                av.argv[i] = av.argv[i + 1];
            }
            av.argc -= 1;

            closeFiles = false;
        }
        else if (strcmp(av.argv[idx], "--nochdir") == 0)
        {
            for (int i = idx; i + 1 < av.argc; ++i)
            {
                av.argv[i] = av.argv[i + 1];
            }
            av.argc -= 1;

            changeDirectory = false;
        }
        else if (strcmp(av.argv[idx], "--pidfile") == 0)
        {
            if (idx + 1 < av.argc)
            {
                pidFile = av.argv[idx + 1];
            }
            else
            {
                if (av.argv[0])
                {
                    consoleErr << av.argv[0] << ": ";
                }
                consoleErr << "--pidfile must be followed by an argument" << endl;
                return EXIT_FAILURE;
            }

            for (int i = idx; i + 2 < av.argc; ++i)
            {
                av.argv[i] = av.argv[i + 2];
            }
            av.argc -= 2;
        }
        else
        {
            ++idx;
        }
    }

    if (!closeFiles && !daemonize)
    {
        if (av.argv[0])
        {
            consoleErr << av.argv[0] << ": ";
        }
        consoleErr << "--noclose must be used with --daemon" << endl;
        return EXIT_FAILURE;
    }

    if (pidFile.size() > 0 && !daemonize)
    {
        if (av.argv[0])
        {
            consoleErr << av.argv[0] << ": ";
        }
        consoleErr << "--pidfile <file> must be used with --daemon" << endl;
        return EXIT_FAILURE;
    }

    if (daemonize)
    {
        configureDaemon(changeDirectory, closeFiles, pidFile);
    }
#endif

    //
    // If no logger has been set yet, we set it to the process logger. If the
    // process logger is the default logger, we change it to a logger which is
    // using the program name for the prefix.
    //
    if (!_logger)
    {
        _logger = getProcessLogger();
        if (dynamic_pointer_cast<LoggerI>(_logger))
        {
            const bool convert = initData.properties->getIcePropertyAsInt("Ice.LogStdErr.Convert") > 0 &&
                                 initData.properties->getIceProperty("Ice.StdErr").empty();

            _logger = make_shared<LoggerI>(initData.properties->getIceProperty("Ice.ProgramName"), "", convert);
            setProcessLogger(_logger);
        }
    }

    return run(av.argc, av.argv, std::move(initData));
}

#ifdef _WIN32
int
Ice::Service::main(int argc, const wchar_t* const argv[], InitializationData initData)
{
    return main(Ice::argsToStringSeq(argc, argv), std::move(initData));
}
#endif

int
Ice::Service::main(const StringSeq& args, InitializationData initData)
{
    IceInternal::ArgVector av(args);
    return main(av.argc, av.argv, std::move(initData));
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

#ifdef _WIN32
int
Ice::Service::run(int argc, const wchar_t* const argv[], InitializationData initData)
{
    StringSeq args = Ice::argsToStringSeq(argc, argv);
    IceInternal::ArgVector av(args);
    return run(av.argc, av.argv, std::move(initData));
}
#endif

int
Ice::Service::run(int argc, const char* const argv[], InitializationData initData)
{
    IceInternal::ArgVector av(argc, argv); // copy args

    if (_service)
    {
#ifdef _WIN32
        return runService(av.argc, av.argv, std::move(initData));
#else
        return runDaemon(av.argc, av.argv, std::move(initData));
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
        ctrlCHandler = new CtrlCHandler;

        //
        // Initialize the communicator.
        //
        _communicator = initializeCommunicator(av.argc, av.argv, std::move(initData));

        //
        // Use the configured logger.
        //
        _logger = _communicator->getLogger();

        //
        // Determines whether we ignore SIGHUP/CTRL_LOGOFF_EVENT.
        //
        _nohup = _communicator->getProperties()->getIcePropertyAsInt("Ice.Nohup") > 0;

        //
        // Start the service.
        //
        if (start(av.argc, av.argv, status))
        {
#ifdef ICE_USE_SYSTEMD
            sd_notify(0, "READY=1");
#endif
            //
            // Wait for service shutdown.
            //
            waitForShutdown();

#ifdef ICE_USE_SYSTEMD
            //
            // Inform the service manager that the service is beginning its shutdown.
            //
            sd_notify(0, "STOPPING=1");
#endif
            //
            // Stop the service.
            //
            if (stop())
            {
                status = EXIT_SUCCESS;
            }
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
#ifdef ICE_USE_SYSTEMD
        const string msg = err.str();
        sd_notifyf(0, "STATUS=Failed service terminating after catching exception: %s", msg.c_str());
#endif
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
#ifdef ICE_USE_SYSTEMD
        const string msg = err.str();
        sd_notifyf(0, "STATUS=Failed service terminating after catching exception: %s", msg.c_str());
#endif
    }
    catch (...)
    {
        error("service terminating after catching unknown exception");
#ifdef ICE_USE_SYSTEMD
        sd_notify(0, "STATUS=Failed service terminating after catching unknown exception");
#endif
    }

    if (_communicator)
    {
        _communicator->destroy();
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
    if (_nohup && sig == CTRL_LOGOFF_EVENT)
    {
        return;
    }
#else
    if (_nohup && sig == SIGHUP)
    {
        return;
    }
#endif

    interrupt();
}

void
Ice::Service::waitForShutdown()
{
    if (_communicator)
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
Ice::Service::initializeCommunicator(int& argc, char* argv[], InitializationData initData)
{
    return Ice::initialize(argc, argv, std::move(initData));
}

void
Ice::Service::syserror(const string& msg)
{
    string errmsg = IceInternal::lastErrorToString();
    if (_logger)
    {
        ostringstream ostr;
        if (!msg.empty())
        {
            ostr << msg << endl;
        }
        if (!errmsg.empty())
        {
            ostr << errmsg;
        }
        _logger->error(ostr.str());
    }
    else
    {
        if (!_name.empty())
        {
            consoleErr << _name << ": ";
        }
        if (!msg.empty())
        {
            consoleErr << msg << endl;
        }
        if (!errmsg.empty())
        {
            consoleErr << errmsg;
        }
    }
}

void
Ice::Service::error(const string& msg)
{
    if (_logger)
    {
        _logger->error(msg);
    }
    else
    {
        if (!_name.empty())
        {
            consoleErr << _name << ": ";
        }
        consoleErr << "error: " << msg << endl;
    }
}

void
Ice::Service::warning(const string& msg)
{
    if (_logger)
    {
        _logger->warning(msg);
    }
    else
    {
        if (!_name.empty())
        {
            consoleErr << _name << ": ";
        }
        consoleErr << "warning: " << msg << endl;
    }
}

void
Ice::Service::trace(const string& msg)
{
    if (_logger)
    {
        _logger->trace("", msg);
    }
    else
    {
        consoleErr << msg << endl;
    }
}

void
Ice::Service::print(const string& msg)
{
    if (_logger)
    {
        _logger->print(msg);
    }
    else
    {
        consoleErr << msg << endl;
    }
}

void
Ice::Service::enableInterrupt()
{
    ctrlCHandler->setCallback(ctrlCHandlerCallback);
}

void
Ice::Service::disableInterrupt()
{
    ctrlCHandler->setCallback(nullptr);
}

#ifdef _WIN32

int
Ice::Service::runService(int argc, const char* const argv[], InitializationData initData)
{
    assert(_service);

    if (_name.empty())
    {
        error("invalid name for Win32 service");
        return EXIT_FAILURE;
    }

    //
    // Arguments passed to the executable are not passed to the service's main function,
    // so save them now and serviceMain will merge them later.
    //
    for (int idx = 1; idx < argc; ++idx)
    {
        _serviceArgs.push_back(argv[idx]);
    }

    _initData = std::move(initData);

    //
    // Don't need to use a wide string converter as the wide string is passed
    // to Windows API.
    //
    const wstring serviceName = stringToWstring(_name, getProcessStringConverter());
    SERVICE_TABLE_ENTRYW ste[] = {
        {const_cast<wchar_t*>(serviceName.c_str()), Ice_Service_ServiceMain},
        {0, 0},
    };

    //
    // Start the service.
    //
    if (!StartServiceCtrlDispatcherW(ste))
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
    if (exitCode != 0)
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
    if (!QueryServiceStatus(hService, &status))
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
    while (status.dwCurrentState == pendingState)
    {
        //
        // Do not wait longer than the wait hint. A good interval is
        // one tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.
        //

        DWORD waitTime = status.dwWaitHint / 10;

        if (waitTime < 1000)
        {
            waitTime = 1000;
        }
        else if (waitTime > 10000)
        {
            waitTime = 10000;
        }

        Sleep(waitTime);

        //
        // Check the status again.
        //
        if (!QueryServiceStatus(hService, &status))
        {
            return false;
        }

        if (status.dwCheckPoint > oldCheckPoint)
        {
            //
            // The service is making progress.
            //
            startTickCount = GetTickCount();
            oldCheckPoint = status.dwCheckPoint;
        }
        else
        {
            if (GetTickCount() - startTickCount > status.dwWaitHint)
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
    switch (status.dwCurrentState)
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

    ServiceTrace tr(this);
    tr << msg << "\n  Current state: " << state << "\n  Exit code: " << status.dwWin32ExitCode
       << "\n  Service specific exit code: " << status.dwServiceSpecificExitCode
       << "\n  Check point: " << status.dwCheckPoint << "\n  Wait hint: " << status.dwWaitHint;
}

void
Ice::Service::serviceMain(int argc, const wchar_t* const argv[])
{
    ctrlCHandler = new CtrlCHandler;

    //
    // Register the control handler function.
    //
    _statusHandle = RegisterServiceCtrlHandlerW(argv[0], Ice_Service_CtrlHandler);
    if (_statusHandle == 0)
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
    // Don't need to pass a wide string converter in the argv conversions
    // as argv come from Windows API.
    //
    const Ice::StringConverterPtr converter = Ice::getProcessStringConverter();

    //
    // Merge the executable's arguments with the service's arguments.
    //
    char** args = new char*[_serviceArgs.size() + argc];

    //
    // First argument is argv[0] the serviceName
    //
    const string serviceName = wstringToString(argv[0], converter);
    args[0] = const_cast<char*>(serviceName.c_str());

    int i = 1;
    for (vector<string>::iterator p = _serviceArgs.begin(); p != _serviceArgs.end(); ++p)
    {
        args[i++] = const_cast<char*>(p->c_str());
    }

    //
    // Convert wide string wchar_t** argv to a sequence of narrow strings and merge
    // the converted sequence into the args array.
    //
    vector<string> executableArgs;
    for (int j = 1; j < argc; ++j)
    {
        executableArgs.push_back(Ice::wstringToString(argv[j], converter));
    }
    for (vector<string>::iterator p = executableArgs.begin(); p != executableArgs.end(); ++p)
    {
        args[i++] = const_cast<char*>(p->c_str());
    }
    argc += static_cast<int>(_serviceArgs.size());

    //
    // If we can't initialize a communicator, then stop immediately.
    //
    try
    {
        _communicator = initializeCommunicator(argc, args, _initData);
    }
    catch (const Ice::Exception& ex)
    {
        delete[] args;
        {
            ServiceError err(this);
            err << "exception occurred while initializing a communicator:\n" << ex;
        }
        terminateService(EXIT_FAILURE);
        return;
    }
    catch (...)
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
    _nohup = _communicator->getProperties()->getIcePropertyAsInt("Ice.Nohup") > 0;

    DWORD status = EXIT_FAILURE;
    try
    {
        int tmpStatus = EXIT_FAILURE;
        if (start(argc, args, tmpStatus))
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
            if (stop())
            {
                status = EXIT_SUCCESS;
            }
        }
        else
        {
            status = tmpStatus;
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
    }
    catch (...)
    {
        error("service terminating after catching unknown exception");
    }

    delete[] args;

    assert(_communicator);
    _communicator->destroy();

    terminateService(status);
}

void
Ice::Service::control(int ctrl)
{
    assert(serviceStatusManager);

    switch (ctrl)
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
            if (ctrl != SERVICE_CONTROL_INTERROGATE)
            {
                ServiceError err(this);
                err << "unrecognized service control code " << ctrl;
            }

            serviceStatusManager->reportStatus();
            break;
        }
    }
}

ServiceStatusManager::ServiceStatusManager(SERVICE_STATUS_HANDLE handle) : _handle(handle), _stopped(false)
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
    lock_guard lock(_mutex);

    assert(state == SERVICE_START_PENDING || state == SERVICE_STOP_PENDING);
    assert(!_thread.joinable());

    _status.dwCurrentState = state;
    _status.dwControlsAccepted = 0; // Don't accept any other control messages while pending.

    _stopped = false;

    _thread = std::thread(&ServiceStatusManager::run, this);
}

void
ServiceStatusManager::stopUpdate()
{
    std::thread thread;

    {
        lock_guard lock(_mutex);

        if (_thread.joinable())
        {
            _stopped = true;
            _conditionVariable.notify_one();
            thread = std::move(_thread);
        }
    }

    thread.join();
}

void
ServiceStatusManager::changeStatus(DWORD state, DWORD controlsAccepted)
{
    lock_guard lock(_mutex);

    _status.dwCurrentState = state;
    _status.dwControlsAccepted = controlsAccepted;

    SetServiceStatus(_handle, &_status);
}

void
ServiceStatusManager::reportStatus()
{
    lock_guard lock(_mutex);

    SetServiceStatus(_handle, &_status);
}

void
ServiceStatusManager::run()
{
    unique_lock lock(_mutex);

    auto delay = chrono::milliseconds(1000);
    _status.dwWaitHint = 2000;
    _status.dwCheckPoint = 0;

    while (!_stopped)
    {
        _status.dwCheckPoint++;
        SetServiceStatus(_handle, &_status);
        _conditionVariable.wait_for(lock, delay);
    }
}

#else

int
Ice::Service::runDaemon(int argc, char* argv[], InitializationData initData)
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
    if (pid < 0)
    {
        if (argv[0])
        {
            consoleErr << argv[0] << ": ";
        }
        consoleErr << IceInternal::errorToString(errno) << endl;
        return EXIT_FAILURE;
    }

    if (pid != 0)
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
        while (true)
        {
            if (read(fds[0], &c, 1) == -1)
            {
                if (IceInternal::interrupted())
                {
                    continue;
                }

                if (argv[0])
                {
                    consoleErr << argv[0] << ": ";
                }
                consoleErr << IceInternal::errorToString(errno) << endl;
                _exit(EXIT_FAILURE);
            }
            break;
        }

        if (c != 0)
        {
            //
            // Read an error message.
            //
            ssize_t rs;
            char s[16];
            string message;
            while ((rs = read(fds[0], &s, 16)) > 0)
            {
                message.append(s, static_cast<size_t>(rs));
            }

            if (argv[0])
            {
                consoleErr << argv[0] << ": ";
            }

            if (rs == -1)
            {
                consoleErr << "I/O error while reading error message from child:\n"
                           << IceInternal::errorToString(errno);
            }
            else
            {
                consoleErr << "failure occurred in daemon";
                if (!message.empty())
                {
                    consoleErr << ":\n" << message;
                }
            }
            consoleErr << endl;
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
        if (setsid() == -1)
        {
            throw SyscallException{__FILE__, __LINE__, "setsid failed", errno};
        }

        //
        // Ignore SIGHUP so that the grandchild process is not sent SIGHUP when this
        // process exits.
        //
        signal(SIGHUP, SIG_IGN); // NOLINT(cert-err33-c)

        //
        // Fork again to eliminate the possibility of acquiring a controlling terminal.
        //
        pid = fork();
        if (pid < 0)
        {
            throw SyscallException{__FILE__, __LINE__, "fork failed", errno};
        }
        if (pid != 0)
        {
            exit(0);
        }

        if (_changeDirectory)
        {
            //
            // Change the working directory.
            //
            if (chdir("/") != 0)
            {
                throw SyscallException{__FILE__, __LINE__, "chdir failed", errno};
            }
        }

        vector<int> fdsToClose;
        if (_closeFiles)
        {
            //
            // Take a snapshot of the open file descriptors. We don't actually close these
            // descriptors until after the communicator is initialized, so that plug-ins
            // have an opportunity to use stdin/stdout/stderr if necessary. This also
            // conveniently allows the Ice.PrintProcessId property to work as expected.
            //
            int fdMax = static_cast<int>(sysconf(_SC_OPEN_MAX));
            if (fdMax <= 0)
            {
                throw SyscallException{__FILE__, __LINE__, "sysconf failed", errno};
            }

            for (int i = 0; i < fdMax; ++i)
            {
                if (fcntl(i, F_GETFL) != -1)
                {
                    //
                    // Don't close the write end of the pipe.
                    //
                    if (i != fds[1])
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
        // be created to monitor connections.
        //
        ctrlCHandler = new CtrlCHandler;

        //
        // Initialize the communicator.
        //
        _communicator = initializeCommunicator(argc, argv, std::move(initData));

        if (_closeFiles)
        {
            //
            // Close unnecessary file descriptors.
            //
            PropertiesPtr properties = _communicator->getProperties();
            string stdOut = properties->getIceProperty("Ice.StdOut");
            string stdErr = properties->getIceProperty("Ice.StdErr");

            for (int& p : fdsToClose)
            {
                //
                // NOTE: Do not close stdout if Ice.StdOut is defined. Likewise for Ice.StdErr.
                //
                if ((p == 1 && !stdOut.empty()) || (p == 2 && !stdErr.empty()))
                {
                    continue;
                }
                close(p);
            }

            //
            // Associate stdin, stdout and stderr with /dev/null.
            //
            int fd = open("/dev/null", O_RDWR);
            assert(fd == 0);
            if (fd != 0)
            {
                throw SyscallException{__FILE__, __LINE__, "open /dev/null failed", errno};
            }
            if (stdOut.empty())
            {
                fd = dup2(0, 1);
                assert(fd == 1);
                if (fd != 1)
                {
                    throw SyscallException{__FILE__, __LINE__, "dup2(0, 1) failed", errno};
                }
            }
            if (stdErr.empty())
            {
                fd = dup2(1, 2);
                assert(fd == 2);
                if (fd != 2)
                {
                    throw SyscallException{__FILE__, __LINE__, "dup2(1, 2) failed", errno};
                }
            }
        }

        //
        // Write PID
        //
        if (_pidFile.size() > 0)
        {
            ofstream of(_pidFile.c_str());
            of << getpid() << endl;

            if (!of)
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
        if (start(argc, argv, status))
        {
            //
            // Notify the parent that the child is ready.
            //
            char c = 0;
            while (true)
            {
                if (write(fds[1], &c, 1) == -1)
                {
                    if (IceInternal::interrupted())
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
            if (stop())
            {
                status = EXIT_SUCCESS;
            }
        }
    }
    catch (const Ice::Exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
        errMsg = err.str();
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "service terminating after catching exception:\n" << ex;
        errMsg = err.str();
    }
    catch (...)
    {
        errMsg = "service terminating after catching unknown exception";
        error(errMsg);
    }

    //
    // If the service failed and the pipe to the parent is still open,
    // then send an error notification to the parent.
    //
    if (status != EXIT_SUCCESS && fds[1] != -1)
    {
        char c = 1;
        while (true)
        {
            if (write(fds[1], &c, 1) == -1)
            {
                if (IceInternal::interrupted())
                {
                    continue;
                }
            }
            break;
        }
        const char* msg = errMsg.c_str();
        size_t len = strlen(msg) + 1; // Include null byte
        size_t pos = 0;
        while (len > 0)
        {
            ssize_t n = write(fds[1], &msg[pos], len);
            if (n == -1)
            {
                if (IceInternal::interrupted())
                {
                    continue;
                }
                else
                {
                    break;
                }
            }
            len -= static_cast<size_t>(n);
            pos += static_cast<size_t>(n);
        }
        close(fds[1]);
    }

    if (_communicator)
    {
        _communicator->destroy();
    }

    return status;
}

#endif
