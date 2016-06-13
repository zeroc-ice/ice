// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class ServiceManagerI : public ServiceManager, 
                        public IceUtil::Monitor<IceUtil::Mutex>,
                        public Ice::EnableSharedFromThis<ServiceManagerI>
{
public:

    ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

    virtual void startService(ICE_IN(std::string), const ::Ice::Current&);
    virtual void stopService(ICE_IN(std::string), const ::Ice::Current&);

    virtual void addObserver(ICE_IN(ServiceObserverPrxPtr), const Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

    int run();

    bool start();
    void stop();

#ifdef ICE_CPP11_MAPPING
    void observerCompleted(const std::shared_ptr<ServiceObserverPrx>&, std::exception_ptr);
#else
    void observerCompleted(const Ice::AsyncResultPtr&);
#endif

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

    void servicesStarted(const std::vector<std::string>&, const std::set<ServiceObserverPrxPtr>&);
    void servicesStopped(const std::vector<std::string>&, const std::set<ServiceObserverPrxPtr>&);

#ifdef ICE_CPP11_MAPPING
    std::function<void(std::exception_ptr)> makeObserverCompletedCallback(const std::shared_ptr<ServiceObserverPrx>&);
    void observerRemoved(const std::shared_ptr<ServiceObserverPrx>&, std::exception_ptr);
#else
    void observerRemoved(const ServiceObserverPrx&, const std::exception&);
#endif

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

    std::set<ServiceObserverPrxPtr> _observers;
    int _traceServiceObserver;
#ifndef ICE_CPP11_MAPPING
    ::Ice::CallbackPtr _observerCompletedCB;
#endif
};
ICE_DEFINE_PTR(ServiceManagerIPtr, ServiceManagerI);

}

#endif
