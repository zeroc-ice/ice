// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
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

    ServiceManagerI(::Ice::CommunicatorPtr);
    virtual ~ServiceManagerI();

    virtual void shutdown(const ::Ice::Current&);

    int run(int&, char*[]);

private:

    bool initServices(int&, char*[]);
    bool startServices();
    void stopServices();

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    std::map<std::string, ServicePtr> _services;
    std::vector< ::IceInternal::DynamicLibraryPtr > _libraries;
};

}

#endif
