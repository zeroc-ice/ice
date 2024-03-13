//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SERVICE_MANAGER_I_H
#define SERVICE_MANAGER_I_H

#include <IceBox/IceBox.h>
#include <Ice/LoggerF.h>
#include <Ice/CommunicatorF.h>
#include <map>

namespace IceBox
{
    class ServiceManagerI;
    using ServiceManagerIPtr = std::shared_ptr<ServiceManagerI>;

    class ServiceManagerI : public ServiceManager, public std::enable_shared_from_this<ServiceManagerI>
    {
    public:
        // Temporary: use public ctor once the implementation uses only the new mapping.
        static ServiceManagerIPtr create(Ice::CommunicatorPtr, int&, char*[]);

        virtual ~ServiceManagerI();

        virtual void startService(std::string, const ::Ice::Current&);
        virtual void stopService(std::string, const ::Ice::Current&);

        virtual void addObserver(std::optional<ServiceObserverPrx>, const Ice::Current&);

        virtual void shutdown(const ::Ice::Current&);

        int run();

        bool start();
        void stop();

        void observerCompleted(const std::optional<ServiceObserverPrx>&, std::exception_ptr);

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
            ::Ice::CommunicatorPtr communicator;
            ::std::string envName;
            ServiceStatus status;
            Ice::StringSeq args;
        };

        ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);

        void start(const std::string&, const std::string&, const ::Ice::StringSeq&);
        void stopAll();

        void servicesStarted(const std::vector<std::string>&, const std::set<std::optional<ServiceObserverPrx>>&);
        void servicesStopped(const std::vector<std::string>&, const std::set<std::optional<ServiceObserverPrx>>&);
        std::function<void(std::exception_ptr)> makeObserverCompletedCallback(const std::optional<ServiceObserverPrx>&);
        void observerRemoved(const std::optional<ServiceObserverPrx>&, std::exception_ptr);

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

        std::set<std::optional<ServiceObserverPrx>> _observers;
        int _traceServiceObserver;

        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
