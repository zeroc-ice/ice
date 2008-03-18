// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/Database.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

//
// Callback from asynchronous call to adapter->getDirectProxy() invoked in LocatorI::findAdapterById_async().
//
class AMI_Adapter_getDirectProxyI : public AMI_Adapter_getDirectProxy
{
public:

    AMI_Adapter_getDirectProxyI(const LocatorI::RequestPtr& request, const string& id) : 
        _request(request), _id(id)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
        assert(obj);
        _request->response(_id, obj);
    }

    virtual void ice_exception(const ::Ice::Exception& e)
    { 
        try
        {
            e.ice_throw();
        }
        catch(const AdapterNotActiveException& ex)
        {
            if(ex.activatable)
            {
                _request->activate(_id);
                return;
            }
        }
        catch(const Ice::Exception&)
        {
        }
        
        _request->exception(_id, e);
    }

private:

    const LocatorI::RequestPtr _request;
    const string _id;
};

class AMI_Adapter_activateI : public AMI_Adapter_activate
{
public:

    AMI_Adapter_activateI(const LocatorIPtr& locator, const string& id) : 
        _locator(locator), _id(id)
    {
    }

    virtual void ice_response(const ::Ice::ObjectPrx& obj)
    {
        _locator->activateFinished(_id, obj);
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        _locator->activateException(_id, ex);
    }

private:

    const LocatorIPtr _locator;
    const string _id;
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
        catch(Ice::AdapterNotFoundException&)
        {
            //
            // We couldn't find the adapter, we ignore and return the
            // original proxy containing the adapter id.
            // 
            _cb->ice_response(_obj);
            return;
        }
        catch(const Ice::Exception& ex)
        {
            //
            // Rethrow unexpected exception.
            //
            _cb->ice_exception(ex);
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

}

LocatorI::Request::Request(const Ice::AMD_Locator_findAdapterByIdPtr& amdCB, 
                           const LocatorIPtr& locator,
                           const string& id,
                           bool replicaGroup,
                           bool roundRobin,
                           const LocatorAdapterInfoSeq& adapters,
                           int count) : 
    _amdCB(amdCB),
    _locator(locator),
    _id(id),
    _replicaGroup(replicaGroup),
    _roundRobin(roundRobin),
    _adapters(adapters),
    _traceLevels(locator->getTraceLevels()),
    _count(count),
    _lastAdapter(_adapters.begin())
{
    assert((_count == 0 && _adapters.empty()) || _count > 0);
}

void
LocatorI::Request::execute()
{
    //
    // If there's no adapters to request, we're done, send the
    // response.
    //
    if(_adapters.empty())
    {
        sendResponse();
        return;
    }

    //
    // Otherwise, request as many adapters as required.
    //
    LocatorAdapterInfoSeq adapters;
    {
        Lock sync(*this);
        assert(_count > 0 && _lastAdapter != _adapters.end());
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
    }
    
    for(LocatorAdapterInfoSeq::const_iterator p = adapters.begin(); p != adapters.end(); ++p)
    {
        p->proxy->getDirectProxy_async(new AMI_Adapter_getDirectProxyI(this, p->id));
    }
}

void
LocatorI::Request::activate(const string& id)
{
    //
    // Activate the adapter
    //
    // NOTE: we use a timeout large enough to ensure that the activate() call won't
    // timeout if the server hangs in deactivation and/or activation.
    //
    for(LocatorAdapterInfoSeq::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
    {
        if(p->id == id)
        {
            _locator->activate(*p, this);
            _activating.insert(id);
        }
    }

    //
    // If this is a request for a replica group, don't wait for the activation to 
    // complete. Instead, we query the next adapter which might be already active.
    //
    if(_replicaGroup)
    {
        LocatorAdapterInfo adapter;
        {
            Lock sync(*this);
            if(_lastAdapter != _adapters.end())
            {
                adapter = *_lastAdapter;
                ++_lastAdapter;
            }
        }
        if(adapter.proxy)
        {
            adapter.proxy->getDirectProxy_async(new AMI_Adapter_getDirectProxyI(this, adapter.id));
        }
    }
}

void
LocatorI::Request::exception(const string& id, const Ice::Exception& ex)
{
    LocatorAdapterInfo adapter;
    {
        Lock sync(*this);
        if(!_exception.get())
        {
            _exception.reset(ex.ice_clone());
        }
        
        _activating.erase(id);

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
        adapter.proxy->getDirectProxy_async(new AMI_Adapter_getDirectProxyI(this, adapter.id));
    }
}

void
LocatorI::Request::response(const string& id, const Ice::ObjectPrx& proxy)
{
    if(!proxy)
    {
        exception(id, AdapterNotActiveException());
        return;
    }

    Lock sync(*this);
    assert(proxy);

    _activating.erase(id);

    _proxies[id] = proxy->ice_identity(_locator->getCommunicator()->stringToIdentity("dummy"));

    //
    // If we received all the required proxies, it's time to send the
    // answer back to the client.
    //
    if(_proxies.size() == _count)
    {
        sendResponse();
    }
}

void
LocatorI::Request::sendResponse()
{
    int roundRobinCount = 0;
    if(_proxies.size() == 1)
    {
        if(_roundRobin)
        {
            for(LocatorAdapterInfoSeq::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
            {
                if(_proxies.find(p->id) != _proxies.end())
                {
                    break;
                }
                //
                // We count the number of object adapters which are inactive until we find
                // one active. This count will be used to update the round robin counter.
                //
                ++roundRobinCount;
            }
        }
        _amdCB->ice_response(_proxies.begin()->second);
    }
    else if(_proxies.empty())
    {
        //
        // If there's no proxies, it's either because we couldn't
        // contact the adapters or because the replica group has
        // no members.
        //
        assert(_exception.get() || (_replicaGroup && _adapters.empty()));

        if(_traceLevels->locator > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "couldn't resolve " << (_replicaGroup ? "replica group `" : "adapter `") << _id << "' endpoints:\n";
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
            else if(_roundRobin && endpoints.empty())
            {
                //
                // We count the number of object adapters which are inactive until we find
                // one active. This count will be used to update the round robin counter.
                //
                ++roundRobinCount;
            }
        }

        for(set<string>::const_iterator q = _activating.begin(); q != _activating.end(); ++q)
        {
            _locator->cancelActivate(*q, this);
        }

        Ice::ObjectPrx proxy = _locator->getCommunicator()->stringToProxy("dummy:default");
        _amdCB->ice_response(proxy->ice_endpoints(endpoints));
    }

    if(_roundRobin)
    {
        _locator->removePendingResolve(_id, roundRobinCount);
    }
}

LocatorI::LocatorI(const Ice::CommunicatorPtr& communicator, 
                   const DatabasePtr& database, 
                   const Ice::LocatorRegistryPrx& locatorRegistry,
                   const RegistryPrx& registry,
                   const QueryPrx& query) :
    _communicator(communicator),
    _database(database),
    _locatorRegistry(locatorRegistry),
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
                               const Ice::Current& current) const
{
    Ice::ObjectPrx proxy;
    try
    {
        proxy = _database->getObjectProxy(id);
    }
    catch(const ObjectNotRegisteredException&)
    {
        throw Ice::ObjectNotFoundException();
    }

    assert(proxy);

    //
    // OPTIMIZATION: If the object is registered with an adapter id,
    // try to get the adapter direct proxy (which might caused the
    // server activation). This will avoid the client to lookup for
    // the adapter id endpoints.
    //
    const string adapterId = proxy->ice_getAdapterId();
    if(!adapterId.empty())
    {
        Ice::AMD_Locator_findAdapterByIdPtr amiCB = new AMD_Locator_findAdapterByIdI(cb, proxy);
        findAdapterById_async(amiCB, adapterId, current);
    }
    else
    {
        cb->ice_response(proxy);
    }
}
    
//
// Find an adapter by identity. The object is searched in the adapter
// registry. If found, we try to get its direct proxy.
//
void
LocatorI::findAdapterById_async(const Ice::AMD_Locator_findAdapterByIdPtr& cb, 
                                const string& id, 
                                const Ice::Current&) const
{
    LocatorIPtr self = const_cast<LocatorI*>(this);
    if(self->addPendingResolve(id, cb))
    {
        //
        // Another request is currently resolving the adapter endpoints. We'll 
        // answer this request once it's done.
        //
        return;
    }

    //
    // If no other request is resolving the adapter endpoints, resolve
    // the endpoints now.
    //
    
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
        _database->getAdapter(id)->getLocatorAdapterInfo(adapters, count, replicaGroup, roundRobin);

        //
        // Round robin replica group requests are serialized. This is
        // required to make sure the round robin counter is accurate
        // even if some adapters are unreachable (bug 2576). For
        // adapters, and replica groups, there's no need to serialize
        // the requests.
        //
        if(!roundRobin)
        {
            self->removePendingResolve(id, 0);
        }

        RequestPtr request = new Request(cb, self, id, replicaGroup, roundRobin, adapters, count);
        request->execute();
    }
    catch(const AdapterNotExistException&)
    {
        try
        {
            cb->ice_response(_database->getAdapterDirectProxy(id));
        }
        catch(const AdapterNotExistException&)
        {
            cb->ice_exception(Ice::AdapterNotFoundException());
        }
        self->removePendingResolve(id, 0);
        return;
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
        self->removePendingResolve(id, 0);
        return;
    }
}

Ice::LocatorRegistryPrx
LocatorI::getRegistry(const Ice::Current&) const
{
    return _locatorRegistry;
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

void
LocatorI::activate(const LocatorAdapterInfo& adapter, const RequestPtr& request)
{
    {
        Lock sync(*this);

        //
        // Check if there's already pending requests for this adapter. If that's the case,
        // we just add this one to the queue. If not, we add it to the queue and initiate
        // a call on the adapter to get its direct proxy.
        //
        PendingRequestsMap::iterator p;
        p = _pendingRequests.insert(make_pair(adapter.id, PendingRequests())).first;
        p->second.insert(request);
        if(p->second.size() != 1)
        {
            return;
        }
    }

    AMI_Adapter_activatePtr amiCB = new AMI_Adapter_activateI(this, adapter.id);
    int timeout = adapter.activationTimeout + adapter.deactivationTimeout;
    AdapterPrx::uncheckedCast(adapter.proxy->ice_timeout(timeout * 1000))->activate_async(amiCB);
}

void
LocatorI::cancelActivate(const string& id, const RequestPtr& request)
{
    Lock sync(*this);
    PendingRequestsMap::iterator p = _pendingRequests.find(id);
    if(p != _pendingRequests.end())
    {
        p->second.erase(request);
    }
}

void
LocatorI::activateFinished(const string& id, const Ice::ObjectPrx& proxy)
{
    PendingRequests requests;
    {
        Lock sync(*this);
        PendingRequestsMap::iterator p = _pendingRequests.find(id);
        assert(p != _pendingRequests.end());
        requests.swap(p->second);
        _pendingRequests.erase(p);
    }

    for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
    {
        (*q)->response(id, proxy);
    }
}

void
LocatorI::activateException(const string& id, const Ice::Exception& ex)
{
    PendingRequests requests;
    {
        Lock sync(*this);
        PendingRequestsMap::iterator p = _pendingRequests.find(id);
        assert(p != _pendingRequests.end());
        requests.swap(p->second);
        _pendingRequests.erase(p);
    }

    for(PendingRequests::iterator q = requests.begin(); q != requests.end(); ++q)
    {
        (*q)->exception(id, ex);
    }
}

bool
LocatorI::addPendingResolve(const string& adapterId, const Ice::AMD_Locator_findAdapterByIdPtr& cb)
{
    Lock sync(*this);
    map<string, deque<Ice::AMD_Locator_findAdapterByIdPtr> >::iterator p = _resolves.find(adapterId);
    if(p == _resolves.end())
    {
        p = _resolves.insert(make_pair(adapterId, deque<Ice::AMD_Locator_findAdapterByIdPtr>())).first;
    }
    else if(p->second.front().get() == cb.get())
    {
        return false;
    }
    
    p->second.push_back(cb);
    return p->second.size() > 1;
}

void
LocatorI::removePendingResolve(const string& adapterId, int roundRobinCount)
{
    Ice::AMD_Locator_findAdapterByIdPtr cb;
    {
        Lock sync(*this);

        //
        // Bump the round robin counter. We bump the round robin counter by 
        // the number of inactive adapters. This ensures that if the first
        // adapters are inactive, if the first adapter to be inactive is the
        // Nth adapter, the next adapter to be returned will be the Nth + 1.
        //
        if(roundRobinCount > 0)
        {
            _database->getAdapter(adapterId)->increaseRoundRobinCount(roundRobinCount);
        }
        
        map<string, deque<Ice::AMD_Locator_findAdapterByIdPtr> >::iterator p = _resolves.find(adapterId);
        assert(p != _resolves.end());
        
        p->second.pop_front();
        if(p->second.empty())
        {
            _resolves.erase(p);
        }
        else
        {
            cb = p->second.front();
        }
    }

    if(cb)
    {
        findAdapterById_async(cb, adapterId);
    }
}
