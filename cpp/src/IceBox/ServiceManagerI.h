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
        ::IceInternal::DynamicLibraryPtr library;
	::Ice::CommunicatorPtr communicator;
        ::Freeze::DBEnvironmentPtr dbEnv;
    };

private:

    void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stop(const std::string&);
    void stopAll();

    ::Ice::Application* _server;
    ::Ice::LoggerPtr _logger;
    std::string _progName; // argv[0]
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    std::map<std::string, ServiceInfo> _services;
};

}

#endif
