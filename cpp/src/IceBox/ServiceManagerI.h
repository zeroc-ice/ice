// Copyright (c) ZeroC, Inc.

#ifndef SERVICE_MANAGER_I_H
#define SERVICE_MANAGER_I_H

#include "Ice/CommunicatorF.h"
#include "Ice/Logger.h"
#include "IceBox/IceBox.h"

namespace IceBox
{
    class ServiceManagerI;
    using ServiceManagerIPtr = std::shared_ptr<ServiceManagerI>;

    class ServiceManagerI final : public ServiceManager, public std::enable_shared_from_this<ServiceManagerI>
    {
    public:
        ServiceManagerI(Ice::CommunicatorPtr, int&, char*[]);

        void startService(std::string, const Ice::Current&) final;
        void stopService(std::string, const Ice::Current&) final;
        bool isServiceRunning(std::string, const Ice::Current&) final;

        void addObserver(std::optional<ServiceObserverPrx>, const Ice::Current&) final;

        void shutdown(const Ice::Current&) final;

        bool start();
        void stop();

        void observerCompleted(const ServiceObserverPrx&, std::exception_ptr);

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
            std::string name;
            ServicePtr service;
            Ice::CommunicatorPtr communicator;
            ServiceStatus status{Stopped};
            Ice::StringSeq args;
        };

        void start(const std::string&, const std::string&, const Ice::StringSeq&);
        void stopAll();

        void addObserver(ServiceObserverPrx);
        void servicesStarted(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&) noexcept;
        void servicesStopped(const std::vector<std::string>&, const std::set<ServiceObserverPrx>&) noexcept;
        std::function<void(std::exception_ptr)> makeObserverCompletedCallback(ServiceObserverPrx);
        void observerRemoved(const ServiceObserverPrx&, std::exception_ptr);

        Ice::PropertiesPtr createServiceProperties(const std::string&);
        void destroyServiceCommunicator(const std::string&, const Ice::CommunicatorPtr&);

        bool configureAdmin(const Ice::PropertiesPtr&, const std::string&);
        void removeAdminFacets(const std::string&);

        Ice::CommunicatorPtr _communicator;
        bool _adminEnabled{false};
        std::set<std::string> _adminFacetFilter;
        Ice::CommunicatorPtr _sharedCommunicator;
        Ice::LoggerPtr _logger;
        Ice::StringSeq _argv; // Filtered server argument vector, not including program name
        std::vector<ServiceInfo> _services;
        bool _pendingStatusChanges{false};

        std::set<ServiceObserverPrx> _observers;
        int _traceServiceObserver{0};

        std::mutex _mutex;
        std::condition_variable _conditionVariable;
    };
}

#endif
