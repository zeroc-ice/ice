//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/Util.h>
#include <IceGrid/SynchronizationException.h>

using namespace std;
using namespace std::chrono;
using namespace IceGrid;

namespace
{

class AdapterRequest final : public LocatorI::Request
{
public:

    AdapterRequest(function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                   function<void(exception_ptr)> exception,
                   const shared_ptr<LocatorI>& locator,
                   const Ice::EncodingVersion& encoding,
                   const LocatorAdapterInfo& adapter) :
        _response(move(response)),
        _exception(move(exception)),
        _locator(locator),
        _encoding(encoding),
        _adapter(adapter),
        _traceLevels(locator->getTraceLevels())
    {
        assert(_adapter.proxy);
    }

    void
    execute() override
    {
        _locator->getDirectProxy(_adapter, shared_from_this());
    }

    void
    activating(const string&) override
    {
        // Nothing to do.
    }

    void
    response(const std::string& id, const shared_ptr<Ice::ObjectPrx>& proxy) override
    {
        assert(proxy);

        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id,
                      make_exception_ptr(Ice::UnsupportedEncodingException(__FILE__, __LINE__,
                                                                           "server doesn't support requested encoding",
                                                                           _encoding,
                                                                           proxy->ice_getEncodingVersion())));
            return;
        }

        _response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
    }

    void
    exception(const std::string&, exception_ptr ex) override
    {
        if(_traceLevels->locator > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "couldn't resolve adapter`" << _adapter.id << "' endpoints:\n" << toString(ex);
        }
        _response(nullptr);
    }

private:

    const function<void(const shared_ptr<Ice::ObjectPrx>&)> _response;
    const function<void(exception_ptr)> _exception;
    const shared_ptr<LocatorI> _locator;
    const Ice::EncodingVersion _encoding;
    const LocatorAdapterInfo _adapter;
    const shared_ptr<TraceLevels> _traceLevels;
};

class ReplicaGroupRequest final : public LocatorI::Request
{
public:

    ReplicaGroupRequest(function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                        function<void(exception_ptr)> exception,
                        const shared_ptr<LocatorI>& locator,
                        const string& id,
                        const Ice::EncodingVersion& encoding,
                        const LocatorAdapterInfoSeq& adapters,
                        int count,
                        shared_ptr<Ice::ObjectPrx> firstProxy) :
        _response(move(response)),
        _exception(move(exception)),
        _locator(locator),
        _id(id),
        _encoding(encoding),
        _adapters(adapters),
        _traceLevels(locator->getTraceLevels()),
        _count(static_cast<unsigned int>(count)),
        _lastAdapter(_adapters.begin())
    {
        assert(_adapters.empty() || _count > 0);

        if(_adapters.empty())
        {
            _count = 0;
        }

        //
        // If the first adapter proxy is provided, store it in _proxies.
        //
        if(firstProxy)
        {
            assert(!_adapters.empty());
            _proxies[_adapters[0].id] = firstProxy;
            ++_lastAdapter;
        }
    }

    void
    execute() override
    {
        //
        // Otherwise, request as many adapters as required.
        //
        LocatorAdapterInfoSeq adapters;
        {
            lock_guard lock(_mutex);
            for(unsigned int i = static_cast<unsigned int>(_proxies.size()); i < _count; ++i)
            {
                if(_lastAdapter == _adapters.end())
                {
                    _count = i;
                    break;
                }
                assert(_lastAdapter->proxy);
                adapters.push_back(*_lastAdapter);
                ++_lastAdapter;
            }

            //
            // If there's no adapters to request, we're done, send the
            // response.
            //
            if(_proxies.size() == _count)
            {
                sendResponse();
                return;
            }
        }

        for(const auto& adapter : adapters)
        {
            if(_locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }
    }

    void
    activating(const string&) override
    {
        //
        // An adapter is being activated. Don't wait for the activation to complete. Instead,
        // we query the next adapter which might be already active.
        //
        LocatorAdapterInfo adapter;
        do
        {
            lock_guard lock(_mutex);
            if(_lastAdapter == _adapters.end())
            {
                break;
            }
            adapter = *_lastAdapter;
            ++_lastAdapter;
        }
        while(_locator->getDirectProxy(adapter, shared_from_this()));
    }

    void
    exception(const string&, exception_ptr exptr) override
    {
        LocatorAdapterInfo adapter;
        {
            lock_guard lock(_mutex);
            if(_proxies.size() == _count) // Nothing to do if we already sent the response.
            {
                return;
            }

            if(!_exptr)
            {
                _exptr = exptr;
            }

            if(_lastAdapter == _adapters.end())
            {
                --_count; // Expect one less adapter proxy if there's no more adapters to query.

                //
                // If we received all the required proxies, it's time to send the
                // answer back to the client.
                //
                if(_count == _proxies.size())
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

        if(adapter.proxy)
        {
            if(_locator->getDirectProxy(adapter, shared_from_this()))
            {
                activating(adapter.id);
            }
        }
    }

    void
    response(const string& id, const shared_ptr<Ice::ObjectPrx>& proxy) override
    {
        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id,
                      make_exception_ptr(Ice::UnsupportedEncodingException(__FILE__,
                                                                           __LINE__,
                                                                           "server doesn't support requested encoding",
                                                                           _encoding,
                                                                           proxy->ice_getEncodingVersion())));
            return;
        }

        lock_guard lock(_mutex);
        assert(proxy);
        if(_proxies.size() == _count) // Nothing to do if we already sent the response.
        {
            return;
        }

        _proxies[id] = proxy->ice_identity(Ice::stringToIdentity("dummy"));

        //
        // If we received all the required proxies, it's time to send the
        // answer back to the client.
        //
        if(_proxies.size() == _count)
        {
            sendResponse();
        }
    }

private:

    void
    sendResponse()
    {
        if(_proxies.size() == 1)
        {
            _response(_proxies.begin()->second);
        }
        else if(_proxies.empty())
        {
            //
            // If there's no proxies, it's either because we couldn't contact the adapters or
            // because the replica group has no members.
            //
            assert(_exptr || _adapters.empty());
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\n";
                out << (_exptr ? toString(_exptr) : string("replica group is empty"));
            }
            _response(nullptr);
        }
        else if(_proxies.size() > 1)
        {
            Ice::EndpointSeq endpoints;
            endpoints.reserve(_proxies.size());
            for(const auto& adapter : _adapters)
            {
                auto q = _proxies.find(adapter.id);
                if(q != _proxies.end())
                {
                    auto edpts = q->second->ice_getEndpoints();
                    endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());

                }
            }

            auto proxy = _locator->getCommunicator()->stringToProxy("dummy:default");
            _response(proxy->ice_endpoints(endpoints));
        }
    }

    const function<void(const shared_ptr<Ice::ObjectPrx>&)> _response;
    const function<void(exception_ptr)> _exception;
    const shared_ptr<LocatorI> _locator;
    const std::string _id;
    const Ice::EncodingVersion _encoding;
    LocatorAdapterInfoSeq _adapters;
    const shared_ptr<TraceLevels> _traceLevels;
    unsigned int _count;
    LocatorAdapterInfoSeq::const_iterator _lastAdapter;
    std::map<std::string, shared_ptr<Ice::ObjectPrx>> _proxies;
    exception_ptr _exptr;
    std::mutex _mutex;
};

class RoundRobinRequest final : public LocatorI::Request, SynchronizationCallback
{
public:

    RoundRobinRequest(function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                      function<void(exception_ptr)> exception,
                      const shared_ptr<LocatorI>& locator,
                      const shared_ptr<Database> database,
                      const string& id,
                      const Ice::Current& current,
                      const LocatorAdapterInfoSeq& adapters,
                      int count) :
        _response(move(response)),
        _exception(move(exception)),
        _locator(locator),
        _database(database),
        _id(id),
        _encoding(current.encoding),
        _connection(current.con),
        _context(current.ctx),
        _adapters(adapters),
        _traceLevels(locator->getTraceLevels()),
        _count(count),
        _waitForActivation(false)
    {
        assert(_adapters.empty() || _count > 0);
    }

    void
    execute() override
    {
        if(_adapters.empty())
        {
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\nreplica group is empty";
            }
            _response(nullptr);
            return;
        }

        LocatorAdapterInfo adapter = _adapters[0];
        assert(adapter.proxy);
        if(_locator->getDirectProxy(adapter, shared_from_this()))
        {
            activating(adapter.id);
        }
    }

    void
    activating(const string& id) override
    {
        LocatorAdapterInfo adapter;
        adapter.id = id;
        do
        {
            lock_guard lock(_mutex);
            if(_adapters.empty() || _waitForActivation)
            {
                return;
            }
            _activatingOrFailed.insert(adapter.id);
            adapter = nextAdapter();
        }
        while(adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()));
    }

    void
    response(const std::string& id, const shared_ptr<Ice::ObjectPrx>& proxy) override
    {
        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id,
                      make_exception_ptr(Ice::UnsupportedEncodingException(__FILE__,
                                                                           __LINE__,
                                                                           "server doesn't support requested encoding",
                                                                           _encoding,
                                                                           proxy->ice_getEncodingVersion())));
            return;
        }

        lock_guard lock(_mutex);
        assert(proxy);
        if(_adapters.empty() || id != _adapters[0].id)
        {
            return;
        }

        if(_count > 1)
        {
            auto p = proxy->ice_identity(Ice::stringToIdentity("dummy"));
            shared_ptr<LocatorI::Request> request =
                make_shared<ReplicaGroupRequest>(_response, _exception, _locator, _id, _encoding, _adapters, _count, p);
            request->execute();
        }
        else
        {
            _response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
        }
        _adapters.clear();
    }

    void
    exception(const std::string& id, exception_ptr ex) override
    {
        LocatorAdapterInfo adapter;
        {
            lock_guard<std::mutex> lock(_mutex);
            _failed.insert(id);
            _activatingOrFailed.insert(id);

            if(!_exptr)
            {
                _exptr = ex;
            }

            if(_adapters.empty() || id != _adapters[0].id)
            {
                return;
            }

            adapter = nextAdapter();
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
        {
            activating(adapter.id);
        }
    }

    void
    synchronized() override
    {
        LocatorAdapterInfo adapter;
        {
            lock_guard lock(_mutex);
            assert(_adapters.empty());
            adapter = nextAdapter();
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
        {
            activating(adapter.id);
        }
    }

    void
    synchronized(exception_ptr ex) override
    {
        LocatorAdapterInfo adapter;
        {
            lock_guard lock(_mutex);
            assert(_adapters.empty());

            if(_activatingOrFailed.size() > _failed.size())
            {
                _waitForActivation = true;
                adapter = nextAdapter();
            }
            else
            {
                if(_traceLevels->locator > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                    out << "couldn't resolve replica group `" << _id << "' endpoints:\n" << toString(ex);
                }
                _response(nullptr);
                return;
            }
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, shared_from_this()))
        {
            activating(adapter.id);
        }
    }

private:

    LocatorAdapterInfo
    nextAdapter()
    {
        bool replicaGroup = false;
        bool roundRobin = false;

        _adapters.clear();

        try
        {
            while(true)
            {
                try
                {
                    if(!_waitForActivation)
                    {
                        _database->getLocatorAdapterInfo(_id, _connection, _context, _adapters, _count, replicaGroup,
                                                         roundRobin, _activatingOrFailed);
                    }

                    if(_waitForActivation || (_adapters.empty() && _activatingOrFailed.size() > _failed.size()))
                    {
                        //
                        // If there are no more adapters to try and some servers were being activated, we
                        // try again but this time we wait for the server activation.
                        //
                        _database->getLocatorAdapterInfo(_id, _connection, _context, _adapters, _count, replicaGroup,
                                                         roundRobin, _failed);
                        _waitForActivation = true;
                    }
                    break;
                }
                catch(const SynchronizationException&)
                {
                    assert(_adapters.empty());
                    bool callback;
                    auto self = dynamic_pointer_cast<SynchronizationCallback>(shared_from_this());
                    assert(self);
                    if(!_waitForActivation)
                    {
                        callback = _database->addAdapterSyncCallback(_id, move(self), _activatingOrFailed);
                    }
                    else
                    {
                        callback = _database->addAdapterSyncCallback(_id, move(self), _failed);
                    }
                    if(callback)
                    {
                        return LocatorAdapterInfo();
                    }
                }
            }

            if(!roundRobin)
            {
                try
                {
                    Ice::Current current;
                    current.encoding = _encoding;
                    _locator->findAdapterByIdAsync( _id, _response, _exception, current);
                }
                catch(const Ice::Exception&)
                {
                    _exception(current_exception());
                }
                _adapters.clear();
                return LocatorAdapterInfo();
            }
            else if(!_adapters.empty())
            {
                return _adapters[0];
            }
            else
            {
                assert(_adapters.empty());
                if(_traceLevels->locator > 0)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                    out << "couldn't resolve replica group `" << _id << "' endpoints:\n";
                    out << (_exptr ? toString(_exptr) : string("replica group is empty"));
                }
                _response(nullptr);
                return LocatorAdapterInfo();
            }
        }
        catch(const AdapterNotExistException&)
        {
            assert(_adapters.empty());
            _exception(make_exception_ptr(Ice::AdapterNotFoundException()));
            return LocatorAdapterInfo();
        }
        catch(const Ice::Exception&)
        {
            assert(_adapters.empty());
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\n" << toString(current_exception());
            }
            _response(nullptr);
            return LocatorAdapterInfo();
        }
    }

    const function<void(const shared_ptr<Ice::ObjectPrx>&)> _response;
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
    bool _waitForActivation;
    set<string> _failed;
    set<string> _activatingOrFailed;
    exception_ptr _exptr;
    mutex _mutex;
};

class FindAdapterByIdCallback final : public SynchronizationCallback
{
public:

    FindAdapterByIdCallback(const shared_ptr<LocatorI>& locator,
                            function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                            function<void(exception_ptr)> exception,
                            const string& id,
                            const Ice::Current& current) :
        _locator(locator),
        _response(move(response)),
        _exception(move(exception)),
        _id(id),
        _current(current)
    {
    }

    void
    synchronized() override
    {
        try
        {
            _locator->findAdapterByIdAsync(_id, _response, _exception, _current);
        }
        catch(const Ice::Exception&)
        {
            _exception(current_exception());
        }
    }

    void
    synchronized(exception_ptr exptr) override
    {
        try
        {
            rethrow_exception(exptr);
        }
        catch(const AdapterNotExistException&)
        {
        }
        catch(const Ice::Exception&)
        {
            const shared_ptr<TraceLevels> traceLevels = _locator->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't resolve adapter `" << _id << "' endpoints:\n" << toString(exptr);
            }
        }
        _response(nullptr);
    }

private:

    const shared_ptr<LocatorI> _locator;
    const function<void(const shared_ptr<Ice::ObjectPrx>&)> _response;
    const function<void(exception_ptr)> _exception;
    const string _id;
    const Ice::Current _current;
};

};

LocatorI::LocatorI(const shared_ptr<Ice::Communicator>& communicator,
                   const shared_ptr<Database>& database,
                   const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
                   const shared_ptr<RegistryPrx>& registry,
                   const shared_ptr<QueryPrx>& query) :
    _communicator(communicator),
    _database(database),
    _wellKnownObjects(wellKnownObjects),
    _localRegistry(registry),
    _localQuery(query)
{
}

//
// Find an object by identity. The object is searched in the object
// registry.
//
void
LocatorI::findObjectByIdAsync(Ice::Identity id,
                              function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
                              function<void(exception_ptr)>,
                              const Ice::Current&) const
{
    try
    {
        response(_database->getObjectProxy(id));
    }
    catch(const ObjectNotRegisteredException&)
    {
        throw Ice::ObjectNotFoundException();
    }
}

//
// Find an adapter by identity. The object is searched in the adapter
// registry. If found, we try to get its direct proxy.
//
void
LocatorI::findAdapterByIdAsync(string id,
                               function<void(const shared_ptr<Ice::ObjectPrx>&)> response,
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
        while(true)
        {
            try
            {
                _database->getLocatorAdapterInfo(id, current.con, current.ctx, adapters, count, replicaGroup,
                                                 roundRobin);
                break;
            }
            catch(const SynchronizationException&)
            {
                if(_database->addAdapterSyncCallback(id, make_shared<FindAdapterByIdCallback>(self, response, exception, id, current)))
                {
                    return;
                }
            }
        }

        shared_ptr<Request> request;
        if(roundRobin)
        {
            request = make_shared<RoundRobinRequest>(response, exception, self, _database, id, current, adapters,
                                                     count);
        }
        else if(replicaGroup)
        {
            request = make_shared<ReplicaGroupRequest>(response, exception, self, id, current.encoding, adapters, count,
                                                       nullptr);
        }
        else
        {
            assert(adapters.size() == 1);
            request = make_shared<AdapterRequest>(response, exception, self, current.encoding, adapters[0]);
        }
        request->execute();
        return;
    }
    catch(const AdapterNotExistException&)
    {
    }
    catch(const Ice::Exception&)
    {
        auto traceLevels = _database->getTraceLevels();
        if(traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            if(replicaGroup)
            {
                out << "couldn't resolve replica group `" << id << "' endpoints:\n" << toString(current_exception());
            }
            else
            {
                out << "couldn't resolve adapter `" << id << "' endpoints:\n" << toString(current_exception());
            }
        }
        response(nullptr);
        return;
    }

    try
    {
        response(_database->getAdapterDirectProxy(id, current.encoding, current.con, current.ctx));
    }
    catch(const AdapterNotExistException&)
    {
        exception(make_exception_ptr(Ice::AdapterNotFoundException()));
    }
}

shared_ptr<Ice::LocatorRegistryPrx>
LocatorI::getRegistry(const Ice::Current&) const
{
    return _wellKnownObjects->getLocatorRegistry();
}

shared_ptr<RegistryPrx>
LocatorI::getLocalRegistry(const Ice::Current&) const
{
    return _localRegistry;
}

shared_ptr<QueryPrx>
LocatorI::getLocalQuery(const Ice::Current&) const
{
    return _localQuery;
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
        if(p != _pendingRequests.end())
        {
            p->second.push_back(request);
            return _activating.find(adapter.id) != _activating.end();
        }

        _pendingRequests.insert({ adapter.id, { request } });
    }

    auto self = shared_from_this();
    adapter.proxy->getDirectProxyAsync([self, adapter] (auto obj)
                                        {
                                            assert(obj);
                                            self->getDirectProxyResponse(adapter, move(obj));
                                        },
                                        [self, adapter] (exception_ptr ex)
                                        {
                                            self->getDirectProxyException(adapter, ex);
                                        });
    return false;
}

void
LocatorI::getDirectProxyResponse(const LocatorAdapterInfo& adapter, const shared_ptr<Ice::ObjectPrx>& proxy)
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

    if(proxy)
    {
        for(const auto& request : requests)
        {
            request->response(adapter.id, proxy);
        }
    }
    else
    {
        for(const auto& request : requests)
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
    catch(const AdapterNotActiveException& e)
    {
        activate = e.activatable;
    }
    catch(const Ice::Exception&)
    {
    }

    PendingRequests requests;
    {
        lock_guard lock(_mutex);
        auto p = _pendingRequests.find(adapter.id);
        assert(p != _pendingRequests.end());
        if(activate)
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

    if(activate)
    {
        for(const auto& request : requests)
        {
            request->activating(adapter.id);
        }

        int timeout = secondsToInt(adapter.activationTimeout + adapter.deactivationTimeout) * 1000;
        auto self = shared_from_this();
        Ice::uncheckedCast<AdapterPrx>(adapter.proxy->ice_invocationTimeout(timeout))->activateAsync(
            [self, adapter] (auto obj)
            {
                self->getDirectProxyResponse(adapter, move(obj));
            },
            [self, adapter] (auto e)
            {
                self->getDirectProxyException(adapter, e);
            });
    }
    else
    {
        for(const auto& request : requests)
        {
            request->exception(adapter.id, ex);
        }
    }
}
