// Copyright (c) ZeroC, Inc.

#include "LocatorI.h"
#include "../Ice/Protocol.h"
#include "Database.h"
#include "Ice/Ice.h"
#include "SessionI.h"
#include "SynchronizationException.h"
#include "Util.h"
#include "WellKnownObjectsManager.h"

using namespace std;
using namespace std::chrono;
using namespace IceGrid;

namespace
{
    class AdapterRequest final : public LocatorI::Request
    {
    public:
        AdapterRequest(
            function<void(const optional<Ice::ObjectPrx>&)> response,
            const shared_ptr<LocatorI>& locator,
            const Ice::EncodingVersion& encoding,
            LocatorAdapterInfo adapter)
            : _response(std::move(response)),
              _locator(locator),
              _encoding(encoding),
              _adapter(std::move(adapter)),
              _traceLevels(locator->getTraceLevels())
        {
            assert(_adapter.proxy);
        }

        void execute() override { _locator->getDirectProxy(_adapter, shared_from_this()); }

        void activating(const string&) final
        {
            // Nothing to do.
        }

        void response(const std::string& id, const Ice::ObjectPrx& proxy) final
        {
            // Ensure the server supports the request encoding.
            if (!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
            {
                exception(
                    id,
                    make_exception_ptr(Ice::FeatureNotSupportedException{
                        __FILE__,
                        __LINE__,
                        "server doesn't support requested encoding " + Ice::encodingVersionToString(_encoding)}));
                return;
            }

            _response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
        }

        void exception(const std::string&, exception_ptr ex) final
        {
            if (_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve adapter '" << _adapter.id << "' endpoints:\n" << toString(ex);
            }
            _response(nullopt);
        }

    private:
        const function<void(const optional<Ice::ObjectPrx>&)> _response;
        const shared_ptr<LocatorI> _locator;
        const Ice::EncodingVersion _encoding;
        const LocatorAdapterInfo _adapter;
        const shared_ptr<TraceLevels> _traceLevels;
    };

    class ReplicaGroupRequest final : public LocatorI::Request
    {
    public:
        ReplicaGroupRequest(
            function<void(const optional<Ice::ObjectPrx>&)> response,
            const shared_ptr<LocatorI>& locator,
            string id,
            const Ice::EncodingVersion& encoding,
            LocatorAdapterInfoSeq adapters,
            int count,
            optional<Ice::ObjectPrx> firstProxy)
            : _response(std::move(response)),
              _locator(locator),
              _id(std::move(id)),
              _encoding(encoding),
              _adapters(std::move(adapters)),
              _traceLevels(locator->getTraceLevels()),
              _count(static_cast<unsigned int>(count)),
              _lastAdapter(_adapters.begin())
        {
            assert(_adapters.empty() || _count > 0);

            if (_adapters.empty())
            {
                _count = 0;
            }

            // If the first adapter proxy is provided, store it in _proxies.
            if (firstProxy)
            {
                assert(!_adapters.empty());
                _proxies.insert({_adapters[0].id, std::move(*firstProxy)});
                ++_lastAdapter;
            }
        }

        void execute() override
        {
            // Otherwise, request as many adapters as required.
            LocatorAdapterInfoSeq adapters;
            {
                lock_guard lock(_mutex);
                for (auto i = static_cast<unsigned int>(_proxies.size()); i < _count; ++i)
                {
                    if (_lastAdapter == _adapters.end())
                    {
                        _count = i;
                        break;
                    }
                    assert(_lastAdapter->proxy);
                    adapters.push_back(*_lastAdapter);
                    ++_lastAdapter;
                }

                // If there's no adapters to request, we're done, send the response.
                if (_proxies.size() == _count)
                {
                    sendResponse();
                    return;
                }
            }

            for (const auto& adapter : adapters)
            {
                if (_locator->getDirectProxy(adapter, shared_from_this()))
                {
                    activating(adapter.id);
                }
            }
        }

        void activating(const string&) override
        {
            //
            // An adapter is being activated. Don't wait for the activation to complete. Instead,
            // we query the next adapter which might be already active.
            //
            LocatorAdapterInfo adapter;
            do
            {
                lock_guard lock(_mutex);
                if (_lastAdapter == _adapters.end())
                {
                    break;
                }
                adapter = *_lastAdapter;
                ++_lastAdapter;
            } while (_locator->getDirectProxy(adapter, shared_from_this()));
        }

        void exception(const string&, exception_ptr exptr) final
        {
            LocatorAdapterInfo adapter;
            {
                lock_guard lock(_mutex);
                if (_proxies.size() == _count) // Nothing to do if we already sent the response.
                {
                    return;
                }

                if (!_exptr)
                {
                    _exptr = exptr;
                }

                if (_lastAdapter == _adapters.end())
                {
                    --_count; // Expect one less adapter proxy if there's no more adapters to query.

                    //
                    // If we received all the required proxies, it's time to send the
                    // answer back to the client.
                    //
                    if (_count == _proxies.size())
                    {
                        sendResponse();
                    }
                }
                else
                {
                    adapter = *_lastAdapter;
                    ++_lastAdapter;
                }
            }

            if (adapter.proxy)
            {
                if (_locator->getDirectProxy(adapter, shared_from_this()))
                {
                    activating(adapter.id);
                }
            }
        }

        void response(const string& id, const Ice::ObjectPrx& proxy) final
        {
            // Ensure the server supports the request encoding.
            if (!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
            {
                exception(
                    id,
                    make_exception_ptr(Ice::FeatureNotSupportedException{
                        __FILE__,
                        __LINE__,
                        "server doesn't support requested encoding " + Ice::encodingVersionToString(_encoding)}));
                return;
            }

            lock_guard lock(_mutex);
            if (_proxies.size() == _count) // Nothing to do if we already sent the response.
            {
                return;
            }

            _proxies.insert({id, proxy->ice_identity(Ice::stringToIdentity("dummy"))});

            // If we received all the required proxies, it's time to send the answer back to the client.
            if (_proxies.size() == _count)
            {
                sendResponse();
            }
        }

    private:
        void sendResponse()
        {
            if (_proxies.size() == 1)
            {
                _response(_proxies.begin()->second);
            }
            else if (_proxies.empty())
            {
                // If there's no proxies, it's either because we couldn't contact the adapters or because the replica
                // group has no members.
                assert(_exptr || _adapters.empty());
                if (_traceLevels->locator > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                    out << "couldn't resolve replica group '" << _id << "' endpoints:\n";
                    out << (_exptr ? toString(_exptr) : string("replica group is empty"));
                }
                _response(nullopt);
            }
            else if (_proxies.size() > 1)
            {
                Ice::EndpointSeq endpoints;
                endpoints.reserve(_proxies.size());
                for (const auto& adapter : _adapters)
                {
                    auto q = _proxies.find(adapter.id);
                    if (q != _proxies.end())
                    {
                        auto edpts = q->second->ice_getEndpoints();
                        endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
                    }
                }

                _response(Ice::ObjectPrx(_locator->getCommunicator(), "dummy:default")->ice_endpoints(endpoints));
            }
        }

        const function<void(const optional<Ice::ObjectPrx>&)> _response;
        const shared_ptr<LocatorI> _locator;
        const std::string _id;
        const Ice::EncodingVersion _encoding;
        LocatorAdapterInfoSeq _adapters;
        const shared_ptr<TraceLevels> _traceLevels;
        unsigned int _count;
        LocatorAdapterInfoSeq::const_iterator _lastAdapter;
        std::map<std::string, Ice::ObjectPrx> _proxies;
        exception_ptr _exptr;
        std::mutex _mutex;
    };

    class RoundRobinRequest final : public LocatorI::Request, SynchronizationCallback
    {
    public:
        RoundRobinRequest(
            function<void(const optional<Ice::ObjectPrx>&)> response,
            function<void(exception_ptr)> exception,
            shared_ptr<LocatorI> locator,
            shared_ptr<Database> database,
            string id,
            const Ice::Current& current,
            LocatorAdapterInfoSeq adapters,
            int count)
            : _response(std::move(response)),
              _exception(std::move(exception)),
              _locator(std::move(locator)),
              _database(std::move(database)),
              _id(std::move(id)),
              _encoding(current.encoding),
              _connection(current.con),
              _context(current.ctx),
              _adapters(std::move(adapters)),
              _traceLevels(_locator->getTraceLevels()),
              _count(count)

        {
            assert(_adapters.empty() || _count > 0);
        }

        void execute() final
        {
            if (_adapters.empty())
            {
                if (_traceLevels->locator > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                    out << "couldn't resolve replica group '" << _id << "' endpoints:\nreplica group is empty";
                }
                _response(nullopt);
                return;
            }

            LocatorAdapterInfo adapter = _adapters[0];
            assert(adapter.proxy);
            if (_locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }

        void activating(const string& id) override
        {
            LocatorAdapterInfo adapter;
            adapter.id = id;
            do
            {
                lock_guard lock(_mutex);
                if (_adapters.empty() || _waitForActivation)
                {
                    return;
                }
                _activatingOrFailed.insert(adapter.id);
                adapter = nextAdapter();
            } while (adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()));
        }

        void response(const std::string& id, const Ice::ObjectPrx& proxy) override
        {
            // Ensure the server supports the request encoding.
            if (!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
            {
                exception(
                    id,
                    make_exception_ptr(Ice::FeatureNotSupportedException{
                        __FILE__,
                        __LINE__,
                        "server doesn't support requested encoding " + Ice::encodingVersionToString(_encoding)}));
                return;
            }

            lock_guard lock(_mutex);
            if (_adapters.empty() || id != _adapters[0].id)
            {
                return;
            }

            if (_count > 1)
            {
                auto request = make_shared<ReplicaGroupRequest>(
                    _response,
                    _locator,
                    _id,
                    _encoding,
                    _adapters,
                    _count,
                    proxy->ice_identity(Ice::stringToIdentity("dummy")));
                request->execute();
            }
            else
            {
                _response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
            }
            _adapters.clear();
        }

        void exception(const std::string& id, exception_ptr ex) override
        {
            LocatorAdapterInfo adapter;
            {
                lock_guard<std::mutex> lock(_mutex);
                _failed.insert(id);
                _activatingOrFailed.insert(id);

                if (!_exptr)
                {
                    _exptr = ex;
                }

                if (_adapters.empty() || id != _adapters[0].id)
                {
                    return;
                }

                adapter = nextAdapter();
            }

            if (adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }

        void synchronized() override
        {
            LocatorAdapterInfo adapter;
            {
                lock_guard lock(_mutex);
                assert(_adapters.empty());
                adapter = nextAdapter();
            }

            if (adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }

        void synchronized(exception_ptr ex) override
        {
            LocatorAdapterInfo adapter;
            {
                lock_guard lock(_mutex);
                assert(_adapters.empty());

                if (_activatingOrFailed.size() > _failed.size())
                {
                    _waitForActivation = true;
                    adapter = nextAdapter();
                }
                else
                {
                    if (_traceLevels->locator > 0)
                    {
                        Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                        out << "couldn't resolve replica group '" << _id << "' endpoints:\n" << toString(ex);
                    }
                    _response(nullopt);
                    return;
                }
            }

            if (adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }

    private:
        LocatorAdapterInfo nextAdapter()
        {
            bool replicaGroup = false;
            bool roundRobin = false;

            _adapters.clear();

            try
            {
                while (true)
                {
                    try
                    {
                        if (!_waitForActivation)
                        {
                            _database->getLocatorAdapterInfo(
                                _id,
                                _connection,
                                _context,
                                _adapters,
                                _count,
                                replicaGroup,
                                roundRobin,
                                _activatingOrFailed);
                        }

                        if (_waitForActivation || (_adapters.empty() && _activatingOrFailed.size() > _failed.size()))
                        {
                            //
                            // If there are no more adapters to try and some servers were being activated, we
                            // try again but this time we wait for the server activation.
                            //
                            _database->getLocatorAdapterInfo(
                                _id,
                                _connection,
                                _context,
                                _adapters,
                                _count,
                                replicaGroup,
                                roundRobin,
                                _failed);
                            _waitForActivation = true;
                        }
                        break;
                    }
                    catch (const SynchronizationException&)
                    {
                        assert(_adapters.empty());
                        bool callback = _waitForActivation
                                            ? _database->addAdapterSyncCallback(
                                                  _id,
                                                  dynamic_pointer_cast<SynchronizationCallback>(shared_from_this()),
                                                  _failed)
                                            : _database->addAdapterSyncCallback(
                                                  _id,
                                                  dynamic_pointer_cast<SynchronizationCallback>(shared_from_this()),
                                                  _activatingOrFailed);

                        if (callback)
                        {
                            return {};
                        }
                    }
                }

                if (!roundRobin)
                {
                    try
                    {
                        Ice::Current current;
                        current.encoding = _encoding;
                        _locator->findAdapterByIdAsync(_id, _response, _exception, current);
                    }
                    catch (const Ice::Exception&)
                    {
                        _exception(current_exception());
                    }
                    _adapters.clear();
                    return {};
                }
                else if (!_adapters.empty())
                {
                    return _adapters[0];
                }
                else
                {
                    assert(_adapters.empty());
                    if (_traceLevels->locator > 0)
                    {
                        Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                        out << "couldn't resolve replica group '" << _id << "' endpoints:\n";
                        out << (_exptr ? toString(_exptr) : string("replica group is empty"));
                    }
                    _response(nullopt);
                    return {};
                }
            }
            catch (const AdapterNotExistException&)
            {
                assert(_adapters.empty());
                _exception(make_exception_ptr(Ice::AdapterNotFoundException()));
                return {};
            }
            catch (const Ice::Exception&)
            {
                assert(_adapters.empty());
                if (_traceLevels->locator > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                    out << "couldn't resolve replica group '" << _id << "' endpoints:\n"
                        << toString(current_exception());
                }
                _response(nullopt);
                return {};
            }
        }

        const function<void(const optional<Ice::ObjectPrx>&)> _response;
        const function<void(exception_ptr)> _exception;
        const shared_ptr<LocatorI> _locator;
        const shared_ptr<Database> _database;
        const string _id;
        const Ice::EncodingVersion _encoding;
        const shared_ptr<Ice::Connection> _connection;
        const Ice::Context _context;
        LocatorAdapterInfoSeq _adapters;
        const shared_ptr<TraceLevels> _traceLevels;
        int _count;
        bool _waitForActivation{false};
        set<string> _failed;
        set<string> _activatingOrFailed;
        exception_ptr _exptr;
        mutex _mutex;
    };

    class FindAdapterByIdCallback final : public SynchronizationCallback
    {
    public:
        FindAdapterByIdCallback(
            const shared_ptr<LocatorI>& locator,
            function<void(const optional<Ice::ObjectPrx>&)> response,
            function<void(exception_ptr)> exception,
            string id,
            Ice::Current current)
            : _locator(locator),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _id(std::move(id)),
              _current(std::move(current))
        {
        }

        void synchronized() override
        {
            try
            {
                _locator->findAdapterByIdAsync(_id, _response, _exception, _current);
            }
            catch (const Ice::Exception&)
            {
                _exception(current_exception());
            }
        }

        void synchronized(exception_ptr exptr) override
        {
            try
            {
                rethrow_exception(exptr);
            }
            catch (const AdapterNotExistException&)
            {
            }
            catch (const Ice::Exception&)
            {
                const shared_ptr<TraceLevels> traceLevels = _locator->getTraceLevels();
                if (traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't resolve adapter '" << _id << "' endpoints:\n" << toString(exptr);
                }
            }
            _response(nullopt);
        }

    private:
        const shared_ptr<LocatorI> _locator;
        const function<void(const optional<Ice::ObjectPrx>&)> _response;
        const function<void(exception_ptr)> _exception;
        const string _id;
        const Ice::Current _current;
    };

};

LocatorI::LocatorI(
    const shared_ptr<Ice::Communicator>& communicator,
    const shared_ptr<Database>& database,
    const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
    RegistryPrx registry,
    QueryPrx query)
    : _communicator(communicator),
      _database(database),
      _wellKnownObjects(wellKnownObjects),
      _localRegistry(std::move(registry)),
      _localQuery(std::move(query))
{
}

//
// Find an object by identity. The object is searched in the object
// registry.
//
void
LocatorI::findObjectByIdAsync(
    Ice::Identity id,
    function<void(const optional<Ice::ObjectPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    try
    {
        response(_database->getObjectProxy(id));
    }
    catch (const ObjectNotRegisteredException&)
    {
        throw Ice::ObjectNotFoundException();
    }
}

//
// Find an adapter by identity. The object is searched in the adapter
// registry. If found, we try to get its direct proxy.
//
void
LocatorI::findAdapterByIdAsync(
    string id,
    function<void(const optional<Ice::ObjectPrx>&)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current) const
{
    auto self = const_pointer_cast<LocatorI>(shared_from_this());

    bool replicaGroup = false;
    try
    {
        //
        // NOTE: getProxies() might throw if the adapter is a server
        // adapter and the node is unreachable (it doesn't throw for
        // replica groups).
        //
        int count;
        LocatorAdapterInfoSeq adapters;
        bool roundRobin;
        while (true)
        {
            try
            {
                _database
                    ->getLocatorAdapterInfo(id, current.con, current.ctx, adapters, count, replicaGroup, roundRobin);
                break;
            }
            catch (const SynchronizationException&)
            {
                if (_database->addAdapterSyncCallback(
                        id,
                        make_shared<FindAdapterByIdCallback>(self, response, exception, id, current)))
                {
                    return;
                }
            }
        }

        shared_ptr<Request> request;
        if (roundRobin)
        {
            request =
                make_shared<RoundRobinRequest>(response, exception, self, _database, id, current, adapters, count);
        }
        else if (replicaGroup)
        {
            request = make_shared<ReplicaGroupRequest>(response, self, id, current.encoding, adapters, count, nullopt);
        }
        else
        {
            assert(adapters.size() == 1);
            request = make_shared<AdapterRequest>(response, self, current.encoding, adapters[0]);
        }
        request->execute();
        return;
    }
    catch (const AdapterNotExistException&)
    {
    }
    catch (const Ice::Exception&)
    {
        auto traceLevels = _database->getTraceLevels();
        if (traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            if (replicaGroup)
            {
                out << "couldn't resolve replica group '" << id << "' endpoints:\n" << toString(current_exception());
            }
            else
            {
                out << "couldn't resolve adapter '" << id << "' endpoints:\n" << toString(current_exception());
            }
        }
        response(nullopt);
        return;
    }

    try
    {
        response(_database->getAdapterDirectProxy(id, current.encoding, current.con, current.ctx));
    }
    catch (const AdapterNotExistException&)
    {
        exception(make_exception_ptr(Ice::AdapterNotFoundException()));
    }
}

void
LocatorI::getRegistryAsync(
    function<void(const optional<Ice::LocatorRegistryPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    response(_wellKnownObjects->getLocatorRegistry());
}

void
LocatorI::getLocalRegistryAsync(
    function<void(const optional<RegistryPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    response(_localRegistry);
}

void
LocatorI::getLocalQueryAsync(
    function<void(const optional<QueryPrx>&)> response,
    function<void(exception_ptr)>,
    const Ice::Current&) const
{
    response(_localQuery);
}

const shared_ptr<Ice::Communicator>&
LocatorI::getCommunicator() const
{
    return _communicator;
}

const shared_ptr<TraceLevels>&
LocatorI::getTraceLevels() const
{
    return _database->getTraceLevels();
}

bool
LocatorI::getDirectProxy(const LocatorAdapterInfo& adapter, const shared_ptr<Request>& request)
{
    {
        lock_guard lock(_mutex);
        auto p = _pendingRequests.find(adapter.id);
        if (p != _pendingRequests.end())
        {
            p->second.push_back(request);
            return _activating.find(adapter.id) != _activating.end();
        }

        _pendingRequests.insert({adapter.id, {request}});
    }

    auto self = shared_from_this();
    adapter.proxy->getDirectProxyAsync(
        [self, adapter](const auto& obj)
        {
            assert(obj);
            self->getDirectProxyResponse(adapter, obj);
        },
        [self, adapter](exception_ptr ex) { self->getDirectProxyException(adapter, ex); });
    return false;
}

void
LocatorI::getDirectProxyResponse(const LocatorAdapterInfo& adapter, const optional<Ice::ObjectPrx>& proxy)
{
    PendingRequests requests;
    {
        lock_guard lock(_mutex);
        auto p = _pendingRequests.find(adapter.id);
        assert(p != _pendingRequests.end());
        requests.swap(p->second);
        _pendingRequests.erase(p);
        _activating.erase(adapter.id);
    }

    if (proxy)
    {
        for (const auto& request : requests)
        {
            request->response(adapter.id, *proxy);
        }
    }
    else
    {
        for (const auto& request : requests)
        {
            request->exception(adapter.id, make_exception_ptr(AdapterNotActiveException()));
        }
    }
}

void
LocatorI::getDirectProxyException(const LocatorAdapterInfo& adapter, exception_ptr ex)
{
    bool activate = false;
    try
    {
        rethrow_exception(ex);
    }
    catch (const AdapterNotActiveException& e)
    {
        activate = e.activatable;
    }
    catch (const Ice::Exception&)
    {
    }

    PendingRequests requests;
    {
        lock_guard lock(_mutex);
        auto p = _pendingRequests.find(adapter.id);
        assert(p != _pendingRequests.end());
        if (activate)
        {
            _activating.insert(adapter.id);
            requests = p->second;
        }
        else
        {
            requests.swap(p->second);
            _pendingRequests.erase(p);
            _activating.erase(adapter.id);
        }
    }

    if (activate)
    {
        for (const auto& request : requests)
        {
            request->activating(adapter.id);
        }

        auto self = shared_from_this();
        adapter.proxy->ice_invocationTimeout(adapter.activationTimeout + adapter.deactivationTimeout)
            ->activateAsync(
                [self, adapter](const auto& obj) { self->getDirectProxyResponse(adapter, obj); },
                [self, adapter](auto e) { self->getDirectProxyException(adapter, e); });
    }
    else
    {
        for (const auto& request : requests)
        {
            request->exception(adapter.id, ex);
        }
    }
}
