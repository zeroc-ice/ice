// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

//
// Callback from asynchronous call to adapter->getDirectProxy() invoked in LocatorI::findAdapterById_async().
//
class AdapterGetDirectProxyCallback : public virtual IceUtil::Shared
{
public:

    AdapterGetDirectProxyCallback(const LocatorIPtr& locator, const LocatorAdapterInfo& adapter) :
        _locator(locator), _adapter(adapter)
    {
    }

    virtual void response(const ::Ice::ObjectPrx& obj)
    {
        assert(obj);
        _locator->getDirectProxyResponse(_adapter, obj);
    }

    virtual void exception(const ::Ice::Exception& e)
    {
        _locator->getDirectProxyException(_adapter, e);
    }

private:

    const LocatorIPtr _locator;
    const LocatorAdapterInfo _adapter;
};

class AdapterActivateCallback : public virtual IceUtil::Shared
{
public:

    AdapterActivateCallback(const LocatorIPtr& locator, const LocatorAdapterInfo& adapter) :
        _locator(locator), _adapter(adapter)
    {
    }

    virtual void response(const ::Ice::ObjectPrx& obj)
    {
        _locator->getDirectProxyResponse(_adapter, obj);
    }

    virtual void exception(const ::Ice::Exception& ex)
    {
        _locator->getDirectProxyException(_adapter, ex);
    }

private:

    const LocatorIPtr _locator;
    const LocatorAdapterInfo _adapter;
};

//
// Callback from asynchrnous call to LocatorI::findAdapterById_async()
// invoked in LocatorI::findObjectById_async().
//
class AMD_Locator_findAdapterByIdI : public Ice::AMD_Locator_findAdapterById
{
public:

    AMD_Locator_findAdapterByIdI(const Ice::AMD_Locator_findObjectByIdPtr& cb, const Ice::ObjectPrx& obj) :
        _cb(cb),
        _obj(obj)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
        //
        // If the adapter dummy direct proxy is not null, return a
        // proxy containing the identity we were looking for and the
        // endpoints of the adapter.
        //
        // If null, return the proxy registered with the object
        // registry.
        //
        if(obj)
        {
            _cb->ice_response(obj->ice_identity(_obj->ice_getIdentity()));
        }
        else
        {
            _cb->ice_response(_obj);
        }
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::AdapterNotFoundException&)
        {
            //
            // We couldn't find the adapter, we ignore and return the
            // original proxy containing the adapter id.
            //
            _cb->ice_response(_obj);
            return;
        }
        catch(const Ice::Exception& e)
        {
            //
            // Rethrow unexpected exception.
            //
            _cb->ice_exception(e);
            return;
        }

        assert(false);
    }

    virtual void ice_exception(const std::exception& ex)
    {
        _cb->ice_exception(ex);
    }

    virtual void ice_exception()
    {
        _cb->ice_exception();
    }

private:

    const Ice::AMD_Locator_findObjectByIdPtr _cb;
    const Ice::ObjectPrx _obj;
};

class AdapterRequest : public LocatorI::Request
{
public:

    AdapterRequest(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB,
                   const LocatorIPtr& locator,
                   const Ice::EncodingVersion& encoding,
                   const LocatorAdapterInfo& adapter) :
        _amdCB(amdCB),
        _locator(locator),
        _encoding(encoding),
        _adapter(adapter),
        _traceLevels(locator->getTraceLevels())
    {
        assert(_adapter.proxy);
    }

    virtual void
    execute()
    {
        _locator->getDirectProxy(_adapter, this);
    }

    virtual void
    activating(const string&)
    {
        // Nothing to do.
    }

    virtual void
    response(const std::string& id, const Ice::ObjectPrx& proxy)
    {
        assert(proxy);

        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id, Ice::UnsupportedEncodingException(__FILE__,
                                                            __LINE__,
                                                            "server doesn't support requested encoding",
                                                            _encoding,
                                                            proxy->ice_getEncodingVersion()));
            return;
        }

        _amdCB->ice_response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
    }

    virtual void
    exception(const std::string&, const Ice::Exception& ex)
    {
        if(_traceLevels->locator > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "couldn't resolve adapter`" << _adapter.id << "' endpoints:\n" << toString(ex);
        }
        _amdCB->ice_response(0);
    }

private:

    const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
    const LocatorIPtr _locator;
    const Ice::EncodingVersion _encoding;
    const LocatorAdapterInfo _adapter;
    const TraceLevelsPtr _traceLevels;
};

class ReplicaGroupRequest : public LocatorI::Request, public IceUtil::Mutex
{
public:

    ReplicaGroupRequest(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB,
                        const LocatorIPtr& locator,
                        const string& id,
                        const Ice::EncodingVersion& encoding,
                        const LocatorAdapterInfoSeq& adapters,
                        int count,
                        Ice::ObjectPrx firstProxy) :
        _amdCB(amdCB),
        _locator(locator),
        _id(id),
        _encoding(encoding),
        _adapters(adapters),
        _traceLevels(locator->getTraceLevels()),
        _count(count),
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

    virtual void
    execute()
    {
        //
        // Otherwise, request as many adapters as required.
        //
        LocatorAdapterInfoSeq adapters;
        {
            Lock sync(*this);
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

        for(LocatorAdapterInfoSeq::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
        {
            if(_locator->getDirectProxy(*p, this))
            {
                activating(p->id);
            }
        }
    }

    virtual void
    activating(const string&)
    {
        //
        // An adapter is being activated. Don't wait for the activation to complete. Instead,
        // we query the next adapter which might be already active.
        //
        LocatorAdapterInfo adapter;
        do
        {
            Lock sync(*this);
            if(_lastAdapter == _adapters.end())
            {
                break;
            }
            adapter = *_lastAdapter;
            ++_lastAdapter;
        }
        while(_locator->getDirectProxy(adapter, this));
    }

    virtual void
    exception(const string& /*id*/, const Ice::Exception& ex)
    {
        LocatorAdapterInfo adapter;
        {
            Lock sync(*this);
            if(_proxies.size() == _count) // Nothing to do if we already sent the response.
            {
                return;
            }

            if(!_exception.get())
            {
                _exception.reset(ex.ice_clone());
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
            if(_locator->getDirectProxy(adapter, this))
            {
                activating(adapter.id);
            }
        }
    }

    virtual void
    response(const string& id, const Ice::ObjectPrx& proxy)
    {
        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id, Ice::UnsupportedEncodingException(__FILE__,
                                                            __LINE__,
                                                            "server doesn't support requested encoding",
                                                            _encoding,
                                                            proxy->ice_getEncodingVersion()));
            return;
        }

        Lock sync(*this);
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
            _amdCB->ice_response(_proxies.begin()->second);
        }
        else if(_proxies.empty())
        {
            //
            // If there's no proxies, it's either because we couldn't contact the adapters or
            // because the replica group has no members.
            //
            assert(_exception.get() || _adapters.empty());
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\n";
                out << (_exception.get() ? toString(*_exception) : string("replica group is empty"));
            }
            _amdCB->ice_response(0);
        }
        else if(_proxies.size() > 1)
        {
            Ice::EndpointSeq endpoints;
            endpoints.reserve(_proxies.size());
            for(LocatorAdapterInfoSeq::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
            {
                map<string, Ice::ObjectPrx>::const_iterator q = _proxies.find(p->id);
                if(q != _proxies.end())
                {
                    Ice::EndpointSeq edpts = q->second->ice_getEndpoints();
                    endpoints.insert(endpoints.end(), edpts.begin(), edpts.end());
                }
            }

            Ice::ObjectPrx proxy = _locator->getCommunicator()->stringToProxy("dummy:default");
            _amdCB->ice_response(proxy->ice_endpoints(endpoints));
        }
    }

    const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
    const LocatorIPtr _locator;
    const std::string _id;
    const Ice::EncodingVersion _encoding;
    LocatorAdapterInfoSeq _adapters;
    const TraceLevelsPtr _traceLevels;
    unsigned int _count;
    LocatorAdapterInfoSeq::const_iterator _lastAdapter;
    std::map<std::string, Ice::ObjectPrx> _proxies;
    IceInternal::UniquePtr<Ice::Exception> _exception;
};

class RoundRobinRequest : public LocatorI::Request, SynchronizationCallback, public IceUtil::Mutex
{
public:

    RoundRobinRequest(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB,
                      const LocatorIPtr& locator,
                      const DatabasePtr database,
                      const string& id,
                      const Ice::Current& current,
                      const LocatorAdapterInfoSeq& adapters,
                      int count) :
        _amdCB(amdCB),
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

    virtual void
    execute()
    {
        if(_adapters.empty())
        {
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\nreplica group is empty";
            }
            _amdCB->ice_response(0);
            return;
        }

        LocatorAdapterInfo adapter = _adapters[0];
        assert(adapter.proxy);
        if(_locator->getDirectProxy(adapter, this))
        {
            activating(adapter.id);
        }
    }

    virtual void
    activating(const string& id)
    {
        LocatorAdapterInfo adapter;
        adapter.id = id;
        do
        {
            Lock sync(*this);
            if(_adapters.empty() || _waitForActivation)
            {
                return;
            }
            _activatingOrFailed.insert(adapter.id);
            adapter = nextAdapter();
        }
        while(adapter.proxy && _locator->getDirectProxy(adapter, this));
    }

    virtual void
    response(const std::string& id, const Ice::ObjectPrx& proxy)
    {
        //
        // Ensure the server supports the request encoding.
        //
        if(!IceInternal::isSupported(_encoding, proxy->ice_getEncodingVersion()))
        {
            exception(id, Ice::UnsupportedEncodingException(__FILE__,
                                                            __LINE__,
                                                            "server doesn't support requested encoding",
                                                            _encoding,
                                                            proxy->ice_getEncodingVersion()));
            return;
        }

        Lock sync(*this);
        assert(proxy);
        if(_adapters.empty() || id != _adapters[0].id)
        {
            return;
        }

        if(_count > 1)
        {
            Ice::ObjectPrx p = proxy->ice_identity(Ice::stringToIdentity("dummy"));
            LocatorI::RequestPtr request =
                new ReplicaGroupRequest(_amdCB, _locator, _id, _encoding, _adapters, _count, p);
            request->execute();
        }
        else
        {
            _amdCB->ice_response(proxy->ice_identity(Ice::stringToIdentity("dummy")));
        }
        _adapters.clear();
    }

    virtual void
    exception(const std::string& id, const Ice::Exception& ex)
    {
        LocatorAdapterInfo adapter;
        {
            Lock sync(*this);
            _failed.insert(id);
            _activatingOrFailed.insert(id);

            if(!_exception.get())
            {
                _exception.reset(ex.ice_clone());
            }

            if(_adapters.empty() || id != _adapters[0].id)
            {
                return;
            }

            adapter = nextAdapter();
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, this))
        {
            activating(adapter.id);
        }
    }

    void
    synchronized()
    {
        LocatorAdapterInfo adapter;
        {
            Lock sync(*this);
            assert(_adapters.empty());
            adapter = nextAdapter();
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, this))
        {
            activating(adapter.id);
        }
    }

    void
    synchronized(const Ice::Exception& ex)
    {
        LocatorAdapterInfo adapter;
        {
            Lock sync(*this);
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
                _amdCB->ice_response(0);
                return;
            }
        }

        if(adapter.proxy && _locator->getDirectProxy(adapter, this))
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
                    if(!_waitForActivation)
                    {
                        callback = _database->addAdapterSyncCallback(_id, this, _activatingOrFailed);
                    }
                    else
                    {
                        callback = _database->addAdapterSyncCallback(_id, this, _failed);
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
                    _locator->findAdapterById_async(_amdCB, _id, current);
                }
                catch(const Ice::Exception& ex)
                {
                    _amdCB->ice_exception(ex);
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
                    out << (_exception.get() ? toString(*_exception) : string("replica group is empty"));
                }
                _amdCB->ice_response(0);
                return LocatorAdapterInfo();
            }
        }
        catch(const AdapterNotExistException&)
        {
            assert(_adapters.empty());
            _amdCB->ice_exception(Ice::AdapterNotFoundException());
            return LocatorAdapterInfo();
        }
        catch(const Ice::Exception& ex)
        {
            assert(_adapters.empty());
            if(_traceLevels->locator > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
                out << "couldn't resolve replica group `" << _id << "' endpoints:\n" << toString(ex);
            }
            _amdCB->ice_response(0);
            return LocatorAdapterInfo();
        }
    }

    const Ice::AMD_Locator_findAdapterByIdPtr _amdCB;
    const LocatorIPtr _locator;
    const DatabasePtr _database;
    const std::string _id;
    const Ice::EncodingVersion _encoding;
    const Ice::ConnectionPtr _connection;
    const Ice::Context _context;
    LocatorAdapterInfoSeq _adapters;
    const TraceLevelsPtr _traceLevels;
    int _count;
    bool _waitForActivation;
    set<string> _failed;
    set<string> _activatingOrFailed;
    IceInternal::UniquePtr<Ice::Exception> _exception;
};

class FindAdapterByIdCallback : public SynchronizationCallback
{
public:

    FindAdapterByIdCallback(const LocatorIPtr& locator,
                            const Ice::AMD_Locator_findAdapterByIdPtr& cb,
                            const string& id,
                            const Ice::Current& current) : _locator(locator), _cb(cb), _id(id), _current(current)
    {
    }

    virtual void
    synchronized()
    {
        try
        {
            _locator->findAdapterById_async(_cb, _id, _current);
        }
        catch(const Ice::Exception& ex)
        {
            _cb->ice_exception(ex);
        }
    }

    virtual void
    synchronized(const Ice::Exception& sex)
    {
        try
        {
            sex.ice_throw();
        }
        catch(const AdapterNotExistException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            const TraceLevelsPtr traceLevels = _locator->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't resolve adapter `" << _id << "' endpoints:\n" << toString(ex);
            }
        }
        _cb->ice_response(0);
    }

private:

    const LocatorIPtr _locator;
    const Ice::AMD_Locator_findAdapterByIdPtr _cb;
    const string _id;
    const Ice::Current _current;
};

};

LocatorI::LocatorI(const Ice::CommunicatorPtr& communicator,
                   const DatabasePtr& database,
                   const WellKnownObjectsManagerPtr& wellKnownObjects,
                   const RegistryPrx& registry,
                   const QueryPrx& query) :
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
LocatorI::findObjectById_async(const Ice::AMD_Locator_findObjectByIdPtr& cb,
                               const Ice::Identity& id,
                               const Ice::Current&) const
{
    try
    {
        cb->ice_response(_database->getObjectProxy(id));
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
LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& cb,
                                const string& id,
                                const Ice::Current& current) const
{
    LocatorIPtr self = const_cast<LocatorI*>(this);
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
                if(_database->addAdapterSyncCallback(id, new FindAdapterByIdCallback(self, cb, id, current)))
                {
                    return;
                }
            }
        }

        RequestPtr request;
        if(roundRobin)
        {
            request = new RoundRobinRequest(cb, self, _database, id, current, adapters, count);
        }
        else if(replicaGroup)
        {
            request = new ReplicaGroupRequest(cb, self, id, current.encoding, adapters, count, 0);
        }
        else
        {
            assert(adapters.size() == 1);
            request = new AdapterRequest(cb, self, current.encoding, adapters[0]);
        }
        request->execute();
        return;
    }
    catch(const AdapterNotExistException&)
    {
    }
    catch(const Ice::Exception& ex)
    {
        const TraceLevelsPtr traceLevels = _database->getTraceLevels();
        if(traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            if(replicaGroup)
            {
                out << "couldn't resolve replica group `" << id << "' endpoints:\n" << toString(ex);
            }
            else
            {
                out << "couldn't resolve adapter `" << id << "' endpoints:\n" << toString(ex);
            }
        }
        cb->ice_response(0);
        return;
    }

    try
    {
        cb->ice_response(_database->getAdapterDirectProxy(id, current.encoding, current.con, current.ctx));
    }
    catch(const AdapterNotExistException&)
    {
        cb->ice_exception(Ice::AdapterNotFoundException());
    }
}

Ice::LocatorRegistryPrx
LocatorI::getRegistry(const Ice::Current&) const
{
    return _wellKnownObjects->getLocatorRegistry();
}

RegistryPrx
LocatorI::getLocalRegistry(const Ice::Current&) const
{
    return _localRegistry;
}

QueryPrx
LocatorI::getLocalQuery(const Ice::Current&) const
{
    return _localQuery;
}

const Ice::CommunicatorPtr&
LocatorI::getCommunicator() const
{
    return _communicator;
}

const TraceLevelsPtr&
LocatorI::getTraceLevels() const
{
    return _database->getTraceLevels();
}

bool
LocatorI::getDirectProxy(const LocatorAdapterInfo& adapter, const RequestPtr& request)
{
    {
        Lock sync(*this);
        PendingRequestsMap::iterator p = _pendingRequests.find(adapter.id);
        if(p != _pendingRequests.end())
        {
            p->second.push_back(request);
            return _activating.find(adapter.id) != _activating.end();
        }

        PendingRequests requests;
        requests.push_back(request);
        _pendingRequests.insert(make_pair(adapter.id, requests));
    }
    adapter.proxy->begin_getDirectProxy(newCallback_Adapter_getDirectProxy(
                                            new AdapterGetDirectProxyCallback(this, adapter),
                                            &AdapterGetDirectProxyCallback::response,
                                            &AdapterGetDirectProxyCallback::exception));
    return false;
}

void
LocatorI::getDirectProxyResponse(const LocatorAdapterInfo& adapter, const Ice::ObjectPrx& proxy)
{
    PendingRequests requests;
    {
        Lock sync(*this);
        PendingRequestsMap::iterator p = _pendingRequests.find(adapter.id);
        assert(p != _pendingRequests.end());
        requests.swap(p->second);
        _pendingRequests.erase(p);
        _activating.erase(adapter.id);
    }

    if(proxy)
    {
        for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
        {
            (*q)->response(adapter.id, proxy);
        }
    }
    else
    {
        for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
        {
            (*q)->exception(adapter.id, AdapterNotActiveException());
        }
    }
}

void
LocatorI::getDirectProxyException(const LocatorAdapterInfo& adapter, const Ice::Exception& ex)
{
    bool activate = false;
    try
    {
        ex.ice_throw();
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
        Lock sync(*this);
        PendingRequestsMap::iterator p = _pendingRequests.find(adapter.id);
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
        for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
        {
            (*q)->activating(adapter.id);
        }

        int timeout = adapter.activationTimeout + adapter.deactivationTimeout;
        AdapterPrx::uncheckedCast(adapter.proxy->ice_invocationTimeout(timeout * 1000))->begin_activate(
            newCallback_Adapter_activate(new AdapterActivateCallback(this, adapter),
                                         &AdapterActivateCallback::response,
                                         &AdapterActivateCallback::exception));
    }
    else
    {
        for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
        {
            (*q)->exception(adapter.id, ex);
        }
    }
}
