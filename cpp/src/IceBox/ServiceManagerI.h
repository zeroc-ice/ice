// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class ServiceManagerI : public ServiceManager, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    virtual void startService(const std::string&, const ::Ice::Current&);
    virtual void stopService(const std::string&, const ::Ice::Current&);

    virtual void addObserver(const ServiceObserverPrx&, const Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

    int run();

    bool start();
    void stop();

    void observerCompleted(const Ice::AsyncResultPtr&);

private:

    enum ServiceStatus
    {
        Stopping,
        Stopped,
        Starting,
        Started
    };

    struct ServiceInfo
    {
        ::std::string name;
        ServicePtr service;
        ::IceInternal::DynamicLibraryPtr library;
        ::Ice::CommunicatorPtr communicator;
        ::std::string envName;
        ServiceStatus status;
        Ice::StringSeq args;
    };

    void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
    void stopAll();

    void servicesStarted(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&);
    void servicesStopped(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&);
    void observerRemoved(const ServiceObserverPrx&, const std::exception&);

    Ice::PropertiesPtr createServiceProperties(const std::string&);
    void destroyServiceCommunicator(const std::string&, const Ice::CommunicatorPtr&);
    
    bool configureAdmin(const Ice::PropertiesPtr&, const std::string&);
    void removeAdminFacets(const std::string&);

    ::Ice::CommunicatorPtr _communicator;
    bool _adminEnabled;
    std::set<std::string> _adminFacetFilter;
    ::Ice::CommunicatorPtr _sharedCommunicator;
    ::Ice::LoggerPtr _logger;
    ::Ice::StringSeq _argv; // Filtered server argument vector, not including program name
    std::vector<ServiceInfo> _services;
    bool _pendingStatusChanges;

    std::set<ServiceObserverPrx> _observers;
    int _traceServiceObserver;
    ::Ice::CallbackPtr _observerCompletedCB;
};

typedef IceUtil::Handle<ServiceManagerI> ServiceManagerIPtr;

}

#endif
