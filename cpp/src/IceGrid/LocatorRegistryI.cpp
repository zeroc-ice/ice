// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

template<class AmdCB>
class SetDirectProxyCB : public LocatorRegistryI::AdapterSetDirectProxyCB
{
public:

    SetDirectProxyCB(const AmdCB& cb,
                     const TraceLevelsPtr& traceLevels,
                     const string& id,
                     const Ice::ObjectPrx& proxy) :
        _cb(cb), _traceLevels(traceLevels), _id(id), _proxy(proxy)
    {
    }

    virtual void response()
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "registered adapter `" << _id << "' endpoints: `";
            out << (_proxy ? _proxy->ice_toString() : string("")) << "'";
        }
        _cb->ice_response();
    }

    virtual void exception(const ::Ice::Exception& ex)
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "failed to register adapter `" << _id << "' endpoints:\n" << ex;
        }

        try
        {
            ex.ice_throw();
        }
        catch(const AdapterActiveException&)
        {
            _cb->ice_exception(Ice::AdapterAlreadyActiveException());
            return;
        }
        catch(const Ice::ObjectNotExistException&)
        {
            _cb->ice_exception(Ice::AdapterNotFoundException()); // Expected if the adapter was destroyed.
            return;
        }
        catch(const Ice::Exception&)
        {
            _cb->ice_exception(Ice::AdapterNotFoundException());
            return;
        }

        assert(false);
    }

private:

    const AmdCB _cb;
    const TraceLevelsPtr _traceLevels;
    const string _id;
    const Ice::ObjectPrx _proxy;
};

template<class AmdCB> SetDirectProxyCB<AmdCB>*
newSetDirectProxyCB(const AmdCB& cb, const TraceLevelsPtr& traceLevels, const string& id, const Ice::ObjectPrx& p)
{
    return new SetDirectProxyCB<AmdCB>(cb, traceLevels, id, p);
}

class ServerSetProcessCB : public virtual IceUtil::Shared
{
public:

    ServerSetProcessCB(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                       const TraceLevelsPtr& traceLevels,
                       const string& id,
                       const Ice::ObjectPrx& proxy) :
        _cb(cb), _traceLevels(traceLevels), _id(id), _proxy(proxy)
    {
    }

    virtual void ice_response()
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "registered server `" << _id << "' process proxy: `";
            out << (_proxy ? _proxy->ice_toString() : string("")) << "'";
        }
        _cb->ice_response();
    }

    virtual void ice_exception(const ::Ice::Exception& ex)
    {
        if(_traceLevels->locator > 1)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->locatorCat);
            out << "failed to register server process proxy `" << _id << "':\n" << ex;
        }

        try
        {
            ex.ice_throw();
        }
        catch(const Ice::ObjectNotExistException&)
        {
            // Expected if the server was destroyed.
            _cb->ice_exception(Ice::ServerNotFoundException());
            return;
        }
        catch(const Ice::LocalException&)
        {
            _cb->ice_exception(Ice::ServerNotFoundException());
            return;
        }

        assert(false);
    }

private:

    const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr _cb;
    const TraceLevelsPtr _traceLevels;
    const string _id;
    const Ice::ObjectPrx _proxy;
};
typedef IceUtil::Handle<ServerSetProcessCB> ServerSetProcessCBPtr;

class SetAdapterDirectProxyCallback : public SynchronizationCallback
{
public:

    SetAdapterDirectProxyCallback(const LocatorRegistryIPtr& registry,
                                  const LocatorRegistryI::AdapterSetDirectProxyCBPtr& amiCB,
                                  const string& adapterId,
                                  const string& replicaGroupId,
                                  const Ice::ObjectPrx& proxy) :
        _registry(registry), _amiCB(amiCB), _adapterId(adapterId), _replicaGroupId(replicaGroupId), _proxy(proxy)
    {
    }

    virtual void
    synchronized()
    {
        try
        {
            _registry->setAdapterDirectProxy(_amiCB, _adapterId, _replicaGroupId, _proxy);
        }
        catch(const Ice::Exception& ex)
        {
            _amiCB->exception(ex);
        }
    }

    virtual void
    synchronized(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::Exception& ex)
        {
            _amiCB->exception(ex);
        }
    }

private:

    const LocatorRegistryIPtr _registry;
    const LocatorRegistryI::AdapterSetDirectProxyCBPtr _amiCB;
    const string _adapterId;
    const string _replicaGroupId;
    const Ice::ObjectPrx _proxy;
};

class SetServerProcessProxyCallback : public SynchronizationCallback
{
public:

    SetServerProcessProxyCallback(const LocatorRegistryIPtr& registry,
                                  const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                  const string& id,
                                  const Ice::ProcessPrx& proxy) :
        _registry(registry), _cb(cb), _id(id), _proxy(proxy)
    {
    }

    virtual void
    synchronized()
    {
        try
        {
            _registry->setServerProcessProxy_async(_cb, _id, _proxy, Ice::Current());
        }
        catch(const Ice::Exception& ex)
        {
            _cb->ice_exception(ex);
        }
    }

    virtual void
    synchronized(const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const ServerNotExistException&)
        {
            _cb->ice_exception(Ice::ServerNotFoundException());
        }
        catch(const Ice::Exception& ex)
        {
            const TraceLevelsPtr traceLevels = _registry->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register server `" << _id << "' process proxy:\n" << toString(ex);
            }
            _cb->ice_exception(Ice::ServerNotFoundException());
        }
    }

private:

    const LocatorRegistryIPtr _registry;
    const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr _cb;
    const string _id;
    const Ice::ProcessPrx _proxy;
};

};

LocatorRegistryI::LocatorRegistryI(const DatabasePtr& database,
                                   bool dynamicRegistration,
                                   bool master,
                                   ReplicaSessionManager& session) :
    _database(database),
    _dynamicRegistration(dynamicRegistration),
    _master(master),
    _session(session)
{
}

void
LocatorRegistryI::setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr& cb,
                                              const string& adapterId,
                                              const Ice::ObjectPrx& proxy,
                                              const Ice::Current&)
{
    setAdapterDirectProxy(newSetDirectProxyCB(cb, _database->getTraceLevels(), adapterId, proxy),
                          adapterId,
                          "",
                          proxy);
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxy_async(
    const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr& cb,
    const string& adapterId,
    const string& replicaGroupId,
    const Ice::ObjectPrx& proxy,
    const Ice::Current&)
{
    setAdapterDirectProxy(newSetDirectProxyCB(cb, _database->getTraceLevels(), adapterId, proxy),
                          adapterId,
                          replicaGroupId,
                          proxy);
}

void
LocatorRegistryI::setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr& cb,
                                              const string& id,
                                              const Ice::ProcessPrx& proxy,
                                              const Ice::Current&)
{
    try
    {
        //
        // Get the server from the registry and set its process proxy.
        //
        // NOTE: We pass false to the getServer call to indicate that
        // we don't necessarily want an up-to-date adapter proxy. This
        // is needed for the session activation mode for cases where
        // the server is released during the server startup.
        //
        ServerPrx server;
        while(true)
        {
            try
            {
                server = _database->getServer(id)->getProxy(false);
                break;
            }
            catch(const SynchronizationException&)
            {
                if(_database->getServer(id)->addSyncCallback(new SetServerProcessProxyCallback(this, cb, id, proxy)))
                {
                    return;
                }
            }
        }

        server->begin_setProcess(proxy, IceGrid::newCallback_Server_setProcess(
                                                new ServerSetProcessCB(cb, _database->getTraceLevels(), id, proxy),
                                                &ServerSetProcessCB::ice_response,
                                                &ServerSetProcessCB::ice_exception));
    }
    catch(const ServerNotExistException&)
    {
        cb->ice_exception(Ice::ServerNotFoundException());
    }
    catch(const Ice::Exception& ex)
    {
        const TraceLevelsPtr traceLevels = _database->getTraceLevels();
        if(traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            out << "couldn't register server `" << id << "' process proxy:\n" << toString(ex);
        }
        cb->ice_exception(Ice::ServerNotFoundException());
    }
}

void
LocatorRegistryI::setAdapterDirectProxy(const LocatorRegistryI::AdapterSetDirectProxyCBPtr& amiCB,
                                        const string& adapterId,
                                        const string& replicaGroupId,
                                        const Ice::ObjectPrx& proxy)
{
    //
    // Ignore request with empty adapter id.
    //
    if(adapterId.empty())
    {
        amiCB->response();
        return;
    }

    int nRetry = 5;
    do
    {
        try
        {
            //
            // Get the adapter from the registry and set its direct proxy.
            //
            AdapterPrx adapter;
            while(true)
            {
                try
                {
                    adapter = _database->getAdapterProxy(adapterId, replicaGroupId, false);
                    if(!adapter)
                    {
                        throw Ice::AdapterNotFoundException();
                    }
                    break;
                }
                catch(const SynchronizationException&)
                {
                    if(_database->addAdapterSyncCallback(adapterId, new SetAdapterDirectProxyCallback(
                                                             this, amiCB, adapterId, replicaGroupId, proxy)))
                    {
                        return;
                    }
                }
            }

            adapter->begin_setDirectProxy(proxy, IceGrid::newCallback_Adapter_setDirectProxy(amiCB,
                                              &LocatorRegistryI::AdapterSetDirectProxyCB::response,
                                              &LocatorRegistryI::AdapterSetDirectProxyCB::exception));
            return;
        }
        catch(const AdapterNotExistException&)
        {
            if(!_dynamicRegistration)
            {
                throw Ice::AdapterNotFoundException();
            }
        }
        catch(const Ice::Exception& ex)
        {
            const TraceLevelsPtr traceLevels = _database->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register adapter `" << adapterId << "' endpoints:\n" << toString(ex);
            }
            throw Ice::AdapterNotFoundException();
        }

        assert(_dynamicRegistration);
        if(_master)
        {
            try
            {
                _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                amiCB->response();
                return;
            }
            catch(const AdapterExistsException&)
            {
                // Continue
            }
            catch(const DeploymentException& ex)
            {
                const TraceLevelsPtr traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n" << ex.reason;
                }
                throw Ice::AdapterNotFoundException();
            }
        }
        else
        {
            ReplicaSessionPrx session = _session.getSession();
            if(!session)
            {
                const TraceLevelsPtr traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n";
                    out << "no session established with the master";
                }
                throw Ice::AdapterNotFoundException();
            }

            try
            {
                session->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                amiCB->response();
                return;
            }
            catch(const AdapterExistsException&)
            {
                // Continue
            }
            catch(const AdapterNotExistException&)
            {
                throw Ice::AdapterNotFoundException(); // Dynamic registration not allowed on the master.
            }
            catch(const Ice::LocalException& ex)
            {
                const TraceLevelsPtr traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n" << toString(ex);
                }
                throw Ice::AdapterNotFoundException();
            }
        }
    }
    while(nRetry-- > 0);
    throw Ice::AdapterNotFoundException();
}

const TraceLevelsPtr&
LocatorRegistryI::getTraceLevels() const
{
    return _database->getTraceLevels();
}
