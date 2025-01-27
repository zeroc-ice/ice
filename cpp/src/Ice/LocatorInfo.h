// Copyright (c) ZeroC, Inc.

#ifndef ICE_LOCATOR_INFO_H
#define ICE_LOCATOR_INFO_H

#include "EndpointIF.h"
#include "Ice/Identity.h"
#include "Ice/Locator.h"
#include "Ice/PropertiesF.h"
#include "Ice/ReferenceF.h"
#include "LocatorInfoF.h"

#include <condition_variable>
#include <mutex>

namespace IceInternal
{
    class LocatorManager final
    {
    public:
        LocatorManager(const Ice::PropertiesPtr&);

        void destroy();

        //
        // Returns locator info for a given locator. Automatically creates
        // the locator info if it doesn't exist yet.
        //
        LocatorInfoPtr get(const Ice::LocatorPrx&);

    private:
        const bool _background;

        using LocatorInfoTable = std::map<Ice::LocatorPrx, LocatorInfoPtr>;
        LocatorInfoTable _table;
        LocatorInfoTable::iterator _tableHint;

        std::map<std::pair<Ice::Identity, Ice::EncodingVersion>, LocatorTablePtr> _locatorTables;
        std::mutex _mutex;
    };

    class LocatorTable final
    {
    public:
        LocatorTable();

        void clear();

        bool getAdapterEndpoints(const std::string&, std::chrono::milliseconds, std::vector<EndpointIPtr>&);
        void addAdapterEndpoints(const std::string&, const std::vector<EndpointIPtr>&);
        std::vector<EndpointIPtr> removeAdapterEndpoints(const std::string&);

        bool getObjectReference(const Ice::Identity&, std::chrono::milliseconds, ReferencePtr&);
        void addObjectReference(const Ice::Identity&, const ReferencePtr&);
        ReferencePtr removeObjectReference(const Ice::Identity&);

    private:
        [[nodiscard]] bool checkTTL(const std::chrono::steady_clock::time_point&, std::chrono::milliseconds) const;

        std::map<std::string, std::pair<std::chrono::steady_clock::time_point, std::vector<EndpointIPtr>>>
            _adapterEndpointsMap;
        std::map<Ice::Identity, std::pair<std::chrono::steady_clock::time_point, ReferencePtr>> _objectMap;
        std::mutex _mutex;
    };

    class LocatorInfo final : public std::enable_shared_from_this<LocatorInfo>
    {
    public:
        class GetEndpointsCallback
        {
        public:
            virtual ~GetEndpointsCallback();
            virtual void setEndpoints(const std::vector<EndpointIPtr>&, bool) = 0;
            virtual void setException(std::exception_ptr) = 0;
        };
        using GetEndpointsCallbackPtr = std::shared_ptr<GetEndpointsCallback>;

        class RequestCallback final
        {
        public:
            RequestCallback(ReferencePtr, std::chrono::milliseconds, GetEndpointsCallbackPtr);

            void response(const LocatorInfoPtr&, const std::optional<Ice::ObjectPrx>&);
            void exception(const LocatorInfoPtr&, std::exception_ptr);

        private:
            const ReferencePtr _reference;
            const std::chrono::milliseconds _ttl;
            const GetEndpointsCallbackPtr _callback;
        };
        using RequestCallbackPtr = std::shared_ptr<RequestCallback>;

        class Request
        {
        public:
            void addCallback(
                const ReferencePtr&,
                const ReferencePtr&,
                std::chrono::milliseconds,
                const GetEndpointsCallbackPtr&);

            void response(const std::optional<Ice::ObjectPrx>&);
            void exception(std::exception_ptr);

        protected:
            Request(LocatorInfoPtr, ReferencePtr);
            virtual ~Request();

            virtual void send() = 0;

            const LocatorInfoPtr _locatorInfo;
            const ReferencePtr _reference;

        private:
            std::mutex _mutex;
            std::vector<RequestCallbackPtr> _callbacks;
            std::vector<ReferencePtr> _wellKnownRefs;
            bool _sent{false};
            bool _response{false};
            std::optional<Ice::ObjectPrx> _proxy;
            std::exception_ptr _exception;
        };
        using RequestPtr = std::shared_ptr<Request>;

        LocatorInfo(Ice::LocatorPrx, LocatorTablePtr, bool);

        void destroy();

        bool operator==(const LocatorInfo&) const;
        bool operator<(const LocatorInfo&) const;

        [[nodiscard]] Ice::LocatorPrx getLocator() const { return _locator; }

        std::optional<Ice::LocatorRegistryPrx> getLocatorRegistry();

        void getEndpoints(const ReferencePtr& ref, std::chrono::milliseconds ttl, const GetEndpointsCallbackPtr& cb)
        {
            getEndpoints(ref, nullptr, ttl, cb);
        }
        void getEndpoints(
            const ReferencePtr&,
            const ReferencePtr&,
            std::chrono::milliseconds,
            const GetEndpointsCallbackPtr&);

        void clearCache(const ReferencePtr&);

    private:
        void getEndpointsException(const ReferencePtr&, std::exception_ptr);
        void getEndpointsTrace(const ReferencePtr&, const std::vector<EndpointIPtr>&, bool);
        void trace(const std::string&, const ReferencePtr&, const std::vector<EndpointIPtr>&);
        void trace(const std::string&, const ReferencePtr&, const ReferencePtr&);

        RequestPtr getAdapterRequest(const ReferencePtr&);
        RequestPtr getObjectRequest(const ReferencePtr&);

        void finishRequest(
            const ReferencePtr&,
            const std::vector<ReferencePtr>&,
            const std::optional<Ice::ObjectPrx>&,
            bool);
        friend class Request;
        friend class RequestCallback;

        const Ice::LocatorPrx _locator;
        std::optional<Ice::LocatorRegistryPrx> _locatorRegistry;
        const LocatorTablePtr _table;
        const bool _background;

        std::map<std::string, RequestPtr> _adapterRequests;
        std::map<Ice::Identity, RequestPtr> _objectRequests;
        std::mutex _mutex;
    };
}

#endif
