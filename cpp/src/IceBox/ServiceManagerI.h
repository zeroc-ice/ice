// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SERVICE_MANAGER_I_H
#define SERVICE_MANAGER_I_H

#include <IceBox/IceBox.h>
#include <Ice/LoggerF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/DynamicLibraryF.h>
#include <Ice/Application.h>
#include <Freeze/DBI.h>
#include <map>

namespace IceBox
{

class ServiceManagerI : public ServiceManager
{
public:

    ServiceManagerI(::Ice::Application*, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual void shutdown(const ::Ice::Current&);

    int run();

    struct ServiceInfo
    {
        ServiceBasePtr service;
        std::string dbEnvName;
        ::IceInternal::DynamicLibraryPtr library;
    };

private:

    void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stop(const std::string&);
    void stopAll();

    struct DBEnvironmentInfo
    {
        ::Freeze::DBEnvironmentPtr dbEnv;
	unsigned int openCount;
    };

    ::Ice::Application* _server;
    ::Ice::LoggerPtr _logger;
    std::string _progName; // argv[0]
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    ::Ice::StringSeq _options; // Server property set converted to command-line options
    std::map<std::string, ServiceInfo> _services;
    std::map<std::string, DBEnvironmentInfo> _dbEnvs;
};

}

#endif
