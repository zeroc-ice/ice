// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SERVICE_H
#define ICE_SERVICE_H

#include <IceUtil/Config.h>
#include <Ice/Ice.h>

#ifdef _WIN32
#   include <winsvc.h>
#endif

namespace Ice
{

class ICE_API Service
{
public:

    Service();
    virtual ~Service();

    //
    // Shutdown the service. The default implementation invokes shutdown()
    // on the communicator.
    //
    virtual bool shutdown();

    //
    // Notify the service about a signal interrupt. The default
    // implementation invokes shutdown().
    //
    virtual void interrupt();

    //
    // The primary entry point for services. This function examines
    // the given argument vector for reserved options and takes the
    // appropriate action. The reserved options are shown below.
    //
    // Win32:
    //
    // --service NAME
    //
    // Unix:
    //
    // --daemon [--nochdir] [--noclose]
    //
    // If --service or --daemon are specified, the program runs as
    // a service, otherwise the program runs as a regular foreground
    // process. Any service-specific (and Ice-specific) options
    // are stripped from argv (just as for Ice::initialize()).
    //
    // The return value is an exit status code: EXIT_FAILURE or
    // EXIT_SUCCESS.
    //
    int main(int&, char*[], const InitializationData& = InitializationData());
    int main(int, char* const [], const Ice::InitializationData& = Ice::InitializationData());

#ifdef _WIN32

    int main(int&, wchar_t*[], const InitializationData& = InitializationData());

#endif

    int main(StringSeq&, const InitializationData& = InitializationData());

    //
    // Returns the communicator created by the service.
    //
    Ice::CommunicatorPtr communicator() const;

    //
    // Returns the Service singleton.
    //
    static Service* instance();

    //
    // Indicates whether the program is running as a Win32 service or
    // Unix daemon.
    //
    bool service() const;

    //
    // Returns the program name. If the program is running as a Win32
    // service, the return value is the service name. Otherwise the
    // return value is the executable name (i.e., argv[0]).
    //
    std::string name() const;

    //
    // Returns true if the operating system supports running the
    // program as a Win32 service or Unix daemon.
    //
    bool checkSystem() const;

    //
    // Alternative entry point for services that use their own
    // command-line options. Instead of invoking main(), the
    // program processes its command-line options and invokes
    // run(). To run as a Win32 service or Unix daemon, the
    // program must first invoke configureService() or
    // configureDaemon(), respectively.
    //
    // The return value is an exit status code: EXIT_FAILURE or
    // EXIT_SUCCESS.
    //
#ifdef _WIN32
    
    int run(int&, wchar_t*[], const InitializationData& = InitializationData());
    
#endif

    int run(int&, char*[], const InitializationData& = InitializationData());

#ifdef _WIN32

    //
    // Configures the program to run as a Win32 service with the
    // given name.
    //
    void configureService(const std::string&);

    static void setModuleHandle(HMODULE);

#else

    //
    // Configures the program to run as a Unix daemon. The first
    // argument indicates whether the daemon should change its
    // working directory to the root directory. The second
    // argument indicates whether extraneous file descriptors are
    // closed. If the value of the last argument is not an empty
    // string, the daemon writes its process ID to the given
    // filename.
    //
    void configureDaemon(bool, bool, const std::string&);

#endif

    //
    // Invoked by the CtrlCHandler.
    //
    virtual void handleInterrupt(int);

protected:

    //
    // Prepare a service for execution, including the creation and
    // activation of object adapters and servants.
    //
    virtual bool start(int, char*[], int&) = 0;

    //
    // Blocks until the service shuts down. The default implementation
    // invokes waitForShutdown() on the communicator.
    //
    virtual void waitForShutdown();

    //
    // Clean up resources after shutting down.
    //
    virtual bool stop();

    //
    // Initialize a communicator.
    //
    virtual Ice::CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&);

    //
    // Log a system error, which includes a description of the
    // current system error code.
    //
    virtual void syserror(const std::string&);

    //
    // Log an error.
    //
    virtual void error(const std::string&);

    //
    // Log a warning.
    //
    virtual void warning(const std::string&);

    //
    // Log trace information.
    //
    virtual void trace(const std::string&);

    //
    // Log a literal message.
    //
    virtual void print(const std::string&);

    //
    // Enable the CtrlCHandler to invoke interrupt() when a signal occurs.
    //
    void enableInterrupt();

    //
    // Ignore signals.
    //
    void disableInterrupt();

    //
    // Log Helpers
    //
    typedef LoggerOutput<Service, Service*, &Service::syserror> ServiceSysError;
    typedef LoggerOutput<Service, Service*, &Service::error> ServiceError;
    typedef LoggerOutput<Service, Service*, &Service::warning> ServiceWarning;
    typedef LoggerOutput<Service, Service*, &Service::trace> ServiceTrace;
    typedef LoggerOutput<Service, Service*, &Service::print> ServicePrint;

private:

    Ice::LoggerPtr _logger;
    Ice::CommunicatorPtr _communicator;
    bool _nohup;
    bool _service;
    std::string _name;
  
    static Service* _instance;

#ifdef _WIN32

    int runService(int, char*[], const InitializationData&);
    void terminateService(DWORD);
    bool waitForServiceState(SC_HANDLE, DWORD, SERVICE_STATUS&);
    void showServiceStatus(const std::string&, SERVICE_STATUS&);

    SERVICE_STATUS_HANDLE _statusHandle;
    std::vector<std::string> _serviceArgs;
    InitializationData _initData;

public:

    void serviceMain(int, wchar_t*[]);
    void control(int);

#else

    int runDaemon(int, char*[], const InitializationData&);

    bool _changeDirectory;
    bool _closeFiles;
    std::string _pidFile;

#endif
};

} // End of namespace Ice

#endif
