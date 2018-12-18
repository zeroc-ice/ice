// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SERVICE_H
#define ICE_SERVICE_H

#include <Ice/Initialize.h>
#include <Ice/LoggerUtil.h>

#ifdef _WIN32
#   include <winsvc.h>
#endif

namespace Ice
{

/**
 * A singleton class comparable to Ice::Application but also provides the low-level, platform-specific
 * initialization and shutdown procedures common to system services.
 * \headerfile Ice/Ice.h
 */
class ICE_API Service
{
public:

    Service();
    virtual ~Service();

    /**
     * Shutdown the service. The default implementation invokes shutdown()
     * on the communicator.
     */
    virtual bool shutdown();

    /**
     * Notify the service about a signal interrupt. The default
     * implementation invokes shutdown().
     */
    virtual void interrupt();

    /**
     * The primary entry point for services. This function examines
     * the given argument vector for reserved options and takes the
     * appropriate action. The reserved options are shown below.
     *
     * Win32:
     *
     * --service NAME
     *
     * Unix:
     *
     * --daemon [--nochdir] [--noclose]
     *
     * If --service or --daemon are specified, the program runs as
     * a service, otherwise the program runs as a regular foreground
     * process. Any service-specific (and Ice-specific) options
     * are stripped from argv (just as for Ice::initialize()).
     *
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
     */
    int main(int argc, const char* const argv[], const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);

#ifdef _WIN32
    /**
     * The primary entry point for services. This function examines
     * the given argument vector for reserved options and takes the
     * appropriate action. The reserved options are shown below.
     *
     * Win32:
     *
     * --service NAME
     *
     * Unix:
     *
     * --daemon [--nochdir] [--noclose]
     *
     * If --service or --daemon are specified, the program runs as
     * a service, otherwise the program runs as a regular foreground
     * process. Any service-specific (and Ice-specific) options
     * are stripped from argv (just as for Ice::initialize()).
     *
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
     */
    int main(int argc, const wchar_t* const argv[], const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);
#endif

    /**
     * The primary entry point for services. This function examines
     * the given argument vector for reserved options and takes the
     * appropriate action. The reserved options are shown below.
     *
     * Win32:
     *
     * --service NAME
     *
     * Unix:
     *
     * --daemon [--nochdir] [--noclose]
     *
     * If --service or --daemon are specified, the program runs as
     * a service, otherwise the program runs as a regular foreground
     * process. Any service-specific (and Ice-specific) options
     * are stripped from argv (just as for Ice::initialize()).
     *
     * @param args The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
     */
    int main(const StringSeq& args, const InitializationData& initData = InitializationData(),
             int version = ICE_INT_VERSION);

    /**
     * Obtains the communicator created by the service.
     * @return The service's communicator.
     */
    Ice::CommunicatorPtr communicator() const;

    /**
     * Obtains the Service singleton.
     * @return A pointer to this service.
     */
    static Service* instance();

    /**
     * Indicates whether the program is running as a Win32 service or Unix daemon.
     * @return True if the program is running as a service, false otherwise.
     */
    bool service() const;

    /**
     * Obtains the program name. If the program is running as a Win32
     * service, the return value is the service name. Otherwise the
     * return value is the executable name (i.e., argv[0]).
     * @return The service name.
     */
    std::string name() const;

    /**
     * Determines whether the operating system supports running the
     * program as a Win32 service or Unix daemon.
     * @return True if the system supports services, false otherwise.
     */
    bool checkSystem() const;

    /**
     * Alternative entry point for services that use their own
     * command-line options. Instead of invoking main(), the
     * program processes its command-line options and invokes
     * run(). To run as a Win32 service or Unix daemon, the
     * program must first invoke configureService() or
     * configureDaemon(), respectively.
     *
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
     */
#ifdef _WIN32
    int run(int argc, const wchar_t* const argv[], const InitializationData& initData = InitializationData(),
            int version = ICE_INT_VERSION);
#endif

    /**
     * Alternative entry point for services that use their own
     * command-line options. Instead of invoking main(), the
     * program processes its command-line options and invokes
     * run(). To run as a Win32 service or Unix daemon, the
     * program must first invoke configureService() or
     * configureDaemon(), respectively.
     *
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
     */
    int run(int argc, const char* const argv[], const InitializationData& initData = InitializationData(),
            int version = ICE_INT_VERSION);

#ifdef _WIN32

    /**
     * Configures the program to run as a Win32 service with the given name.
     * @param name The service name.
     */
    void configureService(const std::string& name);

    /// \cond INTERNAL
    static void setModuleHandle(HMODULE);
    /// \endcond

#else

    /**
     * Configures the program to run as a Unix daemon. The first
     * argument indicates whether the daemon should change its
     * working directory to the root directory. The second
     * argument indicates whether extraneous file descriptors are
     * closed. If the value of the last argument is not an empty
     * string, the daemon writes its process ID to the given
     * filename.
     *
     * @param changeDirectory True if the daemon should change its working directory to the root directory,
     * false otherwise.
     * @param closeFiles True if the daemon should close unnecessary file descriptors (i.e., stdin, stdout, etc.),
     * false otherwise.
     * @param pidFile If a non-empty string is provided, the daemon writes its process ID to the given file.
     */
    void configureDaemon(bool changeDirectory, bool closeFiles, const std::string& pidFile);

#endif

    /**
     * Invoked by the signal handler when it catches a signal.
     * @param sig The signal that was caught.
     */
    virtual void handleInterrupt(int sig);

protected:

    /**
     * Prepares a service for execution, including the creation and
     * activation of object adapters and servants.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param status The exit status, which is returned by main
     * @return True if startup was successful, false otherwise.
     */
    virtual bool start(int argc, char* argv[], int& status) = 0;

    /**
     * Blocks until the service shuts down. The default implementation
     * invokes waitForShutdown() on the communicator.
     */
    virtual void waitForShutdown();

    /**
     * Cleans up resources after shutting down.
     */
    virtual bool stop();

    /**
     * Initializes a communicator.
     * @param argc Specifies the number of arguments in argv.
     * @param argv The command-line arguments.
     * @param initData Configuration data for the new Communicator.
     * @param version Indicates the Ice version with which the application is compatible. If not
     * specified, the version of the Ice installation is used.
     * @return The new communicator instance.
     */
    virtual Ice::CommunicatorPtr initializeCommunicator(int& argc, char* argv[], const InitializationData& initData,
                                                        int version);

    /**
     * Logs a system error, which includes a description of the
     * current system error code.
     * @param msg The log message.
     */
    virtual void syserror(const std::string& msg);

    /**
     * Logs an error.
     * @param msg The log message.
     */
    virtual void error(const std::string& msg);

    /**
     * Logs a warning.
     * @param msg The log message.
     */
    virtual void warning(const std::string& msg);

    /**
     * Logs trace information.
     * @param msg The log message.
     */
    virtual void trace(const std::string& msg);

    /**
     * Logs a literal message.
     * @param msg The log message.
     */
    virtual void print(const std::string& msg);

    /**
     * Enables the signal handler to invoke interrupt() when a signal occurs.
     */
    void enableInterrupt();

    /**
     * Ignore signals.
     */
    void disableInterrupt();

    /**
     * Logger utility class for a system error.
     */
    typedef LoggerOutput<Service, Service*, &Service::syserror> ServiceSysError;

    /**
     * Logger utility class for an error.
     */
    typedef LoggerOutput<Service, Service*, &Service::error> ServiceError;

    /**
     * Logger utility class for a warning.
     */
    typedef LoggerOutput<Service, Service*, &Service::warning> ServiceWarning;

    /**
     * Logger utility class for a trace message.
     */
    typedef LoggerOutput<Service, Service*, &Service::trace> ServiceTrace;

    /**
     * Logger utility class for a literal message.
     */
    typedef LoggerOutput<Service, Service*, &Service::print> ServicePrint;

private:

    Ice::LoggerPtr _logger;
    Ice::CommunicatorPtr _communicator;
    bool _nohup;
    bool _service;
    std::string _name;

    static Service* _instance;

#ifdef _WIN32

    int runService(int, const char* const[], const InitializationData&);
    void terminateService(DWORD);
    bool waitForServiceState(SC_HANDLE, DWORD, SERVICE_STATUS&);
    void showServiceStatus(const std::string&, SERVICE_STATUS&);

    SERVICE_STATUS_HANDLE _statusHandle;
    std::vector<std::string> _serviceArgs;
    InitializationData _initData;

public:

    /// \cond INTERNAL
    void serviceMain(int, const wchar_t* const[]);
    void control(int);
    /// \endcond

#else

    int runDaemon(int, char*[], const InitializationData&, int);

    bool _changeDirectory;
    bool _closeFiles;
    std::string _pidFile;

#endif
};

} // End of namespace Ice

#endif
