// Copyright (c) ZeroC, Inc.

#ifndef ICE_SERVICE_H
#define ICE_SERVICE_H

#include "Ice.h"

#ifdef _WIN32
#    ifndef NOMINMAX
#        define NOMINMAX
#    endif
#    include <windows.h>
#    include <winsvc.h>
#endif

namespace Ice
{
    /// A singleton class that helps you write Windows services and Unix daemons using Ice.
    /// @headerfile Ice/Ice.h
    class ICE_API Service
    {
    public:
        /// Default constructor.
        Service();

        virtual ~Service();

        /// Shuts down the service. The default implementation calls Communicator::shutdown.
        virtual bool shutdown();

        /// Notifies the service that the program received a signal. The default implementation calls #shutdown.
        /// @see Ice::CtrlCHandler
        virtual void interrupt();

        /// The primary entry point for services. This function examines @p argv for reserved options and takes the
        /// appropriate action. The reserved options are shown below.
        ///
        /// Win32:
        ///
        /// --service NAME
        ///
        /// Unix:
        ///
        /// --daemon [--nochdir] [--noclose]
        ///
        /// If --service or --daemon are specified, the program runs as a service, otherwise the program runs as a
        /// regular foreground process. Any service-specific (and Ice-specific) options are stripped from @p argv (just
        /// as for ::Ice::initialize).
        /// @param argc The number of arguments in @p argv.
        /// @param argv The command-line arguments.
        /// @param initData Configuration data for the new communicator.
        /// @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
        int main(int argc, const char* const argv[], InitializationData initData = {});

#if defined(_WIN32) || defined(ICE_DOXYGEN)
        /// @copydoc main(int, const char* const[], InitializationData)
        /// @remarks Windows only.
        int main(int argc, const wchar_t* const argv[], InitializationData initData = {});
#endif

        /// The primary entry point for services. This function examines @p args for reserved options and takes the
        /// appropriate action. The reserved options are shown below.
        ///
        /// Win32:
        ///
        /// --service NAME
        ///
        /// Unix:
        ///
        /// --daemon [--nochdir] [--noclose]
        ///
        /// If --service or --daemon are specified, the program runs as a service, otherwise the program runs as a
        /// regular foreground process. Any service-specific (and Ice-specific) options are stripped from @p args (just
        /// as for ::Ice::initialize).
        /// @param args The command-line arguments.
        /// @param initData Configuration data for the new communicator.
        /// @return The application's exit status: EXIT_FAILURE or EXIT_SUCCESS.
        int main(const StringSeq& args, InitializationData initData = {});

        /// Gets the communicator created by the service.
        /// @return The service's communicator.
        [[nodiscard]] Ice::CommunicatorPtr communicator() const;

        /// Gets the Service singleton.
        /// @return A pointer to this service.
        static Service* instance();

        /// Indicates whether the program is running as a Win32 service or Unix daemon.
        /// @return `true` if the program is running as a service, `false` otherwise.
        [[nodiscard]] bool service() const;

        /// Gets the program name. If the program is running as a Win32 service, the return value is the service name.
        /// Otherwise the return value is the executable name (i.e., argv[0]).
        /// @return The service name.
        [[nodiscard]] std::string name() const;

        /// Handles a signal.
        /// @remarks The default implementation calls #interrupt unless the signal is SIGHUP or CTRL_LOGOFF_EVENT and
        /// the property `Ice.Nohup` is greater than 0 (or not set at all).
        /// @param sig The signal that was caught.
        virtual void handleInterrupt(int sig);

#ifdef _WIN32
        /// @private
        static void setModuleHandle(HMODULE);

        /// @private
        void serviceMain(int, const wchar_t* const[]);

        /// @private
        void control(int);
#endif

    protected:
        /// Prepares a service for execution, including the creation and activation of object adapters and servants.
        /// @param argc The number of arguments in @p argv.
        /// @param argv The command-line arguments.
        /// @param[out] status The exit status, which is returned by #main when `start` or `stop` returns `false`.
        /// @return `true` if startup was successful, `false` otherwise.
        virtual bool start(int argc, char* argv[], int& status) = 0;

        /// Blocks until the service shuts down. The default implementation calls Communicator::waitForShutdown.
        virtual void waitForShutdown();

        /// Cleans up resources after shutting down.
        virtual bool stop();

        /// Initializes a communicator.
        /// @param argc The number of arguments in @p argv.
        /// @param argv The command-line arguments.
        /// @param initData Configuration data for the new communicator.
        /// @return The new communicator instance.
        virtual Ice::CommunicatorPtr initializeCommunicator(int& argc, char* argv[], InitializationData initData);

        /// Logs a system error, which includes a description of the current system error code.
        /// @param msg The log message.
        virtual void syserror(const std::string& msg);

        /// Logs an error.
        /// @param msg The log message.
        virtual void error(const std::string& msg);

        /// Logs a warning.
        /// @param msg The log message.
        virtual void warning(const std::string& msg);

        /// Logs trace information.
        /// @param msg The log message.
        virtual void trace(const std::string& msg);

        /// Logs a literal message.
        /// @param msg The log message.
        virtual void print(const std::string& msg);

        /// Makes this service call #handleInterrupt when it receives a signal.
        void enableInterrupt();

        /// Ignores signals.
        void disableInterrupt();

        /// Logger utility class for a system error.
        using ServiceSysError = LoggerOutput<Service, Service*, &Service::syserror>;

        /// Logger utility class for an error.
        using ServiceError = LoggerOutput<Service, Service*, &Service::error>;

        /// Logger utility class for a warning.
        using ServiceWarning = LoggerOutput<Service, Service*, &Service::warning>;

        /// Logger utility class for a trace message.
        using ServiceTrace = LoggerOutput<Service, Service*, &Service::trace>;

        /// Logger utility class for a literal message.
        using ServicePrint = LoggerOutput<Service, Service*, &Service::print>;

    private:
        Ice::LoggerPtr _logger;
        Ice::CommunicatorPtr _communicator;
        bool _nohup = true;
        bool _service = false;
        std::string _name;

        static Service* _instance;

        int run(int argc, const char* const argv[], InitializationData initData = {});

#ifdef _WIN32
        /// Configures the program to run as a Windows service.
        void configureService(const std::string& name);

        int runService(int, const char* const[], InitializationData);
        void terminateService(DWORD);
        bool waitForServiceState(SC_HANDLE, DWORD, SERVICE_STATUS&);
        void showServiceStatus(const std::string&, SERVICE_STATUS&);

        SERVICE_STATUS_HANDLE _statusHandle;
        std::vector<std::string> _serviceArgs;
        InitializationData _initData;
#else
        /// Configures the program to run as a Unix daemon.
        /// @param changeDirectory `true` if the daemon should change its working directory to the root directory,
        /// `false` otherwise.
        /// @param closeFiles `true` if the daemon should close unnecessary file descriptors (i.e., stdin, stdout,
        /// etc.), `false` otherwise.
        /// @param pidFile If a non-empty string is provided, the daemon writes its process ID to the given file.
        /// @remarks Linux and macOS only.
        void configureDaemon(bool changeDirectory, bool closeFiles, const std::string& pidFile);

        int runDaemon(int, char*[], InitializationData);

        bool _changeDirectory = true;
        bool _closeFiles = true;
        std::string _pidFile;
#endif
    };

} // End of namespace Ice

#endif
