// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_LOCATOR_I_H
#define ICEGRID_LOCATOR_I_H

#include "IceGrid/Registry.h"
#include "Internal.h"

#include <set>

namespace IceGrid
{
    class Database;
    class LocatorI;
    class TraceLevels;
    class WellKnownObjectsManager;

    struct LocatorAdapterInfo;
    using LocatorAdapterInfoSeq = std::vector<LocatorAdapterInfo>;

    class LocatorI : public AsyncLocator, public std::enable_shared_from_this<LocatorI>
    {
    public:
        class Request : public std::enable_shared_from_this<Request>
        {
        public:
            virtual void execute() = 0;
            virtual void activating(const std::string&) = 0;
            virtual void response(const std::string&, const Ice::ObjectPrx&) = 0;
            virtual void exception(const std::string&, std::exception_ptr) = 0;
        };

        LocatorI(
            const Ice::CommunicatorPtr&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            RegistryPrx,
            QueryPrx);

        void findObjectByIdAsync(
            Ice::Identity,
            std::function<void(const std::optional<Ice::ObjectPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const override;

        void findAdapterByIdAsync(
            std::string,
            std::function<void(const std::optional<Ice::ObjectPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const override;

        void getRegistryAsync(
            std::function<void(const std::optional<Ice::LocatorRegistryPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const override;

        void getLocalRegistryAsync(
            std::function<void(const std::optional<RegistryPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const override;

        void getLocalQueryAsync(
            std::function<void(const std::optional<QueryPrx>&)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) const override;

        [[nodiscard]] const Ice::CommunicatorPtr& getCommunicator() const;
        [[nodiscard]] const std::shared_ptr<TraceLevels>& getTraceLevels() const;

        bool getDirectProxy(const LocatorAdapterInfo&, const std::shared_ptr<Request>&);
        void getDirectProxyResponse(const LocatorAdapterInfo&, const std::optional<Ice::ObjectPrx>&);
        void getDirectProxyException(const LocatorAdapterInfo&, std::exception_ptr);

    protected:
        const Ice::CommunicatorPtr _communicator;
        const std::shared_ptr<Database> _database;
        const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        const RegistryPrx _localRegistry;
        const QueryPrx _localQuery;

        using PendingRequests = std::vector<std::shared_ptr<Request>>;
        using PendingRequestsMap = std::map<std::string, PendingRequests>;
        PendingRequestsMap _pendingRequests;
        std::set<std::string> _activating;

        std::mutex _mutex;
    };
}

#endif
