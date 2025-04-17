// Copyright (c) ZeroC, Inc.

#ifndef LOOKUPI_H
#define LOOKUPI_H

#include "../Ice/Timer.h"
#include "Ice/Properties.h"
#include "IceDiscovery/Lookup.h"
#include "LocatorI.h"

#include <chrono>
#include <set>

namespace IceDiscovery
{
    class LookupI;

    class Request : public IceInternal::TimerTask
    {
    public:
        Request(LookupIPtr, int);

        virtual bool retry();
        void invoke(const std::string&, const std::vector<std::pair<LookupPrx, LookupReplyPrx>>&);
        bool exception();
        [[nodiscard]] std::string getRequestId() const;

        virtual void finished(const std::optional<Ice::ObjectPrx>&) = 0;

    protected:
        virtual void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&) = 0;

        LookupIPtr _lookup;
        const std::string _requestId;
        int _retryCount;
        size_t _lookupCount{0};
        size_t _failureCount{0};
    };
    using RequestPtr = std::shared_ptr<Request>;

    template<class T, class CB> class RequestT : public Request
    {
    public:
        RequestT(const LookupIPtr& lookup, T id, int retryCount) : Request(lookup, retryCount), _id(std::move(id)) {}

        [[nodiscard]] T getId() const { return _id; }

        bool addCallback(const CB& cb)
        {
            _callbacks.push_back(cb);
            return _callbacks.size() == 1;
        }

        void finished(const std::optional<Ice::ObjectPrx>& proxy) override
        {
            for (const auto& callback : _callbacks)
            {
                callback.first(proxy);
            }
            _callbacks.clear();
        }

    protected:
        const T _id;
        std::vector<CB> _callbacks;
    };

    using ObjectCB =
        std::pair<std::function<void(const std::optional<Ice::ObjectPrx>&)>, std::function<void(std::exception_ptr)>>;
    using AdapterCB =
        std::pair<std::function<void(const std::optional<Ice::ObjectPrx>&)>, std::function<void(std::exception_ptr)>>;

    class ObjectRequest : public RequestT<Ice::Identity, ObjectCB>, public std::enable_shared_from_this<ObjectRequest>
    {
    public:
        ObjectRequest(const LookupIPtr&, const Ice::Identity&, int);

        void response(const Ice::ObjectPrx&);

    private:
        void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&) override;
        void runTimerTask() override;
    };
    using ObjectRequestPtr = std::shared_ptr<ObjectRequest>;

    class AdapterRequest final : public RequestT<std::string, AdapterCB>,
                                 public std::enable_shared_from_this<AdapterRequest>
    {
    public:
        AdapterRequest(const LookupIPtr&, const std::string&, int);

        bool response(const Ice::ObjectPrx&, bool);

        bool retry() final;
        void finished(const std::optional<Ice::ObjectPrx>&) final;

    private:
        void invokeWithLookup(const std::string&, const LookupPrx&, const LookupReplyPrx&) final;
        void runTimerTask() final;

        //
        // We use a set because the same IceDiscovery plugin might return multiple times
        // the same proxy if it's accessible through multiple network interfaces and if we
        // also sent the request to multiple interfaces.
        //
        std::set<Ice::ObjectPrx> _proxies;
        std::chrono::steady_clock::time_point _start;
        std::chrono::nanoseconds _latency;
    };
    using AdapterRequestPtr = std::shared_ptr<AdapterRequest>;

    class LookupI final : public Lookup, public std::enable_shared_from_this<LookupI>
    {
    public:
        LookupI(LocatorRegistryIPtr, const LookupPrx&, const Ice::PropertiesPtr&);

        void destroy();

        void setLookupReply(const LookupReplyPrx&);

        void findObjectById(std::string, Ice::Identity, std::optional<LookupReplyPrx>, const Ice::Current&) final;
        void findAdapterById(std::string, std::string, std::optional<LookupReplyPrx>, const Ice::Current&) final;
        void findObject(const ObjectCB&, const Ice::Identity&);
        void findAdapter(const AdapterCB&, const std::string&);

        void foundObject(const Ice::Identity&, const std::string&, const Ice::ObjectPrx&);
        void foundAdapter(const std::string&, const std::string&, const Ice::ObjectPrx&, bool);

        void adapterRequestTimedOut(const AdapterRequestPtr&);
        void adapterRequestException(const AdapterRequestPtr&, std::exception_ptr);
        void objectRequestTimedOut(const ObjectRequestPtr&);
        void objectRequestException(const ObjectRequestPtr&, std::exception_ptr);

        const IceInternal::TimerPtr& timer() { return _timer; }

        int latencyMultiplier() { return _latencyMultiplier; }

    private:
        LocatorRegistryIPtr _registry;
        LookupPrx _lookup;
        std::vector<std::pair<LookupPrx, LookupReplyPrx>> _lookups;
        const std::chrono::milliseconds _timeout;
        const int _retryCount;
        const int _latencyMultiplier;
        const std::string _domainId;

        IceInternal::TimerPtr _timer;
        bool _warnOnce{true};

        std::map<Ice::Identity, ObjectRequestPtr> _objectRequests;
        std::map<std::string, AdapterRequestPtr> _adapterRequests;
        std::mutex _mutex;
    };

    class LookupReplyI final : public LookupReply
    {
    public:
        LookupReplyI(LookupIPtr);

        void foundObjectById(Ice::Identity, std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
        void foundAdapterById(std::string, std::optional<Ice::ObjectPrx>, bool, const Ice::Current&) final;

    private:
        const LookupIPtr _lookup;
    };
}

#endif
