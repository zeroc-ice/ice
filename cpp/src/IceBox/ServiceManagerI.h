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
#include <map>

namespace IceBox
{

class ServiceManagerI : public ServiceManager
{
public:

    ServiceManagerI(::Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual void shutdown(const ::Ice::Current&);

    int run();

    struct ServiceInfo
    {
        ServicePtr service;
        ::IceInternal::DynamicLibraryPtr library;
    };

private:

    ServicePtr init(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stop(const std::string&);
    void stopAll();

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    std::string _progName; // argv[0]
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    ::Ice::StringSeq _options; // Server property set converted to command-line options
    std::map<std::string, ServiceInfo> _services;
};

}

#endif
