//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
                        public IceUtil::Monitor<IceUtil::Mutex>
                      , public std::enable_shared_from_this<ServiceManagerI>
{
public:

    ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);
    virtual ~ServiceManagerI();

    virtual void startService(std::string, const ::Ice::Current&);
    virtual void stopService(std::string, const ::Ice::Current&);

    virtual void addObserver(ServiceObserverPrxPtr, const Ice::Current&);

    virtual void shutdown(const ::Ice::Current&);

    int run();

    bool start();
    void stop();

    void observerCompleted(const std::shared_ptr<ServiceObserverPrx>&, std::exception_ptr);

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

    std::function<void(std::exception_ptr)> makeObserverCompletedCallback(const std::shared_ptr<ServiceObserverPrx>&);
    void observerRemoved(const std::shared_ptr<ServiceObserverPrx>&, std::exception_ptr);

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
};
ICE_DEFINE_PTR(ServiceManagerIPtr, ServiceManagerI);

}

#endif
