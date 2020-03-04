//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/Database.h>
#include <IceGrid/Util.h>
#include <IceGrid/SynchronizationException.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

tuple<function<void()>, function<void(exception_ptr)>>
newSetDirectProxyCB(function<void()> responseCb,
                    function<void(exception_ptr)> exceptionCb,
                    const shared_ptr<TraceLevels>& traceLevels, const string& id,
                    const shared_ptr<Ice::ObjectPrx>& proxy)
{
    auto response = [traceLevels, id, proxy, responseCb = move(responseCb)] ()
    {
        if(traceLevels->locator > 1)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            out << "registered adapter `" << id << "' endpoints: `";
            out << (proxy ? proxy->ice_toString() : string("")) << "'";
        }
        responseCb();
    };

    auto exception = [traceLevels, id, exceptionCb = move(exceptionCb)](auto exptr)
    {
        if(traceLevels->locator > 1)
        {
            try
            {
                rethrow_exception(exptr);
            }
            catch(const std::exception& ex)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "failed to register adapter `" << id << "' endpoints:\n" << ex;
            }
        }

        try
        {
            rethrow_exception(exptr);
        }
        catch(const AdapterActiveException&)
        {
            exceptionCb(make_exception_ptr(Ice::AdapterAlreadyActiveException()));
            return;
        }
        catch(const Ice::ObjectNotExistException&)
        {
            exceptionCb(make_exception_ptr(Ice::AdapterNotFoundException())); // Expected if the adapter was destroyed).
            return;
        }
        catch(const Ice::Exception&)
        {
            exceptionCb(make_exception_ptr(Ice::AdapterNotFoundException()));
            return;
        }
    };

    return { move(response), move(exception) };
}

class SetAdapterDirectProxyCallback final : public SynchronizationCallback
{
public:

    SetAdapterDirectProxyCallback(const shared_ptr<LocatorRegistryI>& registry,
                                  function<void()> response,
                                  function<void(exception_ptr)> exception,
                                  const string& adapterId,
                                  const string& replicaGroupId,
                                  const shared_ptr<Ice::ObjectPrx>& proxy) :
        _registry(registry),
        _response(move(response)),
        _exception(move(exception)),
        _adapterId(adapterId),
        _replicaGroupId(replicaGroupId),
        _proxy(proxy)
    {
    }

    void
    synchronized() override
    {
        try
        {
            _registry->setAdapterDirectProxy(_adapterId, _replicaGroupId, _proxy, _response, _exception);
        }
        catch(const Ice::Exception&)
        {
            _exception(current_exception());
        }
    }

    void
    synchronized(exception_ptr ex) override
    {
        _exception(ex);
    }

private:

    const shared_ptr<LocatorRegistryI> _registry;
    const function<void()> _response;
    const function<void(exception_ptr)> _exception;
    const string _adapterId;
    const string _replicaGroupId;
    const shared_ptr<Ice::ObjectPrx> _proxy;
};

class SetServerProcessProxyCallback final : public SynchronizationCallback
{
public:

    SetServerProcessProxyCallback(const shared_ptr<LocatorRegistryI>& registry,
                                  const function<void()> response,
                                  const function<void(exception_ptr)> exception,
                                  const string& id,
                                  const shared_ptr<Ice::ProcessPrx>& proxy) :
        _registry(registry),
        _response(move(response)),
        _exception(move(exception)),
        _id(id),
        _proxy(proxy)
    {
    }

    void
    synchronized() override
    {
        try
        {
            _registry->setServerProcessProxyAsync(_id, _proxy, _response, _exception, Ice::Current());
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
        catch(const ServerNotExistException&)
        {
            _exception(make_exception_ptr(Ice::ServerNotFoundException()));
        }
        catch(const Ice::Exception&)
        {
            auto traceLevels = _registry->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register server `" << _id << "' process proxy:\n" << toString(current_exception());
            }
            _exception(make_exception_ptr(Ice::ServerNotFoundException()));
        }
    }

private:

    const shared_ptr<LocatorRegistryI> _registry;
    const function<void()> _response;
    const function<void(exception_ptr)> _exception;
    const string _id;
    const shared_ptr<Ice::ProcessPrx> _proxy;
};

};

LocatorRegistryI::LocatorRegistryI(const shared_ptr<Database>& database,
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
LocatorRegistryI::setAdapterDirectProxyAsync(string adapterId, shared_ptr<Ice::ObjectPrx> proxy,
                                             function<void()> response,
                                             function<void(exception_ptr)> exception,
                                              const Ice::Current&)
{
    auto [responseCb, exceptionCb] = newSetDirectProxyCB(move(response), move(exception), _database->getTraceLevels(),
                                                     adapterId, proxy);

    setAdapterDirectProxy(adapterId,
                          "",
                          proxy,
                          move(responseCb),
                          move(exceptionCb));
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxyAsync(string adapterId, string replicaGroupId,
                                                       shared_ptr<Ice::ObjectPrx> proxy,
                                                       function<void()> response,
                                                       function<void(exception_ptr)> exception,
                                                       const Ice::Current&)
{
    auto [responseCb, exceptionCb] = newSetDirectProxyCB(move(response), move(exception), _database->getTraceLevels(),
                                                     adapterId, proxy);
    setAdapterDirectProxy(adapterId,
                          replicaGroupId,
                          proxy,
                          move(responseCb),
                          move(exceptionCb));
}

void
LocatorRegistryI::setServerProcessProxyAsync(string id, shared_ptr<Ice::ProcessPrx> proxy,
                                             function<void()> response,
                                             function<void(exception_ptr)> exception,
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
        shared_ptr<ServerPrx> server;
        while(true)
        {
            try
            {
                server = _database->getServer(id)->getProxy(false);
                break;
            }
            catch(const SynchronizationException&)
            {
                auto cb = make_shared<SetServerProcessProxyCallback>(shared_from_this(), response,
                                                                     exception, id, proxy);
                if(_database->getServer(id)->addSyncCallback(move(cb)))
                {
                    return;
                }
            }
        }

        server->setProcessAsync(proxy,
                                [id, proxy, response, traceLevels = _database->getTraceLevels()]
                                {
                                    if(traceLevels->locator > 1)
                                    {
                                        Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                                        out << "registered server `" << id << "' process proxy: `";
                                        out << (proxy ? proxy->ice_toString() : string("")) << "'";
                                    }
                                    response();
                                },
                                [id, exception, traceLevels = _database->getTraceLevels()] (exception_ptr exptr)
                                {
                                    if(traceLevels->locator > 1)
                                    {
                                        try
                                        {
                                            rethrow_exception(exptr);
                                        }
                                        catch(const std::exception& ex)
                                        {
                                            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                                            out << "failed to register server process proxy `" << id << "':\n" << ex;
                                        }
                                    }

                                    try
                                    {
                                        rethrow_exception(exptr);
                                    }
                                    catch(const Ice::ObjectNotExistException&)
                                    {
                                        // Expected if the server was destroyed.
                                        exception(make_exception_ptr(Ice::ServerNotFoundException()));
                                        return;
                                    }
                                    catch(const Ice::LocalException&)
                                    {
                                        exception(make_exception_ptr(Ice::ServerNotFoundException()));
                                        return;
                                    }
                                });
    }
    catch(const ServerNotExistException&)
    {
        exception(make_exception_ptr(Ice::ServerNotFoundException()));
    }
    catch(const Ice::Exception&)
    {
        auto traceLevels = _database->getTraceLevels();
        if(traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            out << "couldn't register server `" << id << "' process proxy:\n" << toString(current_exception());
        }
        exception(make_exception_ptr(Ice::ServerNotFoundException()));
    }
}

void
LocatorRegistryI::setAdapterDirectProxy(string adapterId, string replicaGroupId,
                                        shared_ptr<Ice::ObjectPrx> proxy,
                                        function<void()> response,
                                        function<void(exception_ptr)> exception)
{
    //
    // Ignore request with empty adapter id.
    //
    if(adapterId.empty())
    {
        response();
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
            shared_ptr<AdapterPrx> adapter;
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
                    if(_database->addAdapterSyncCallback(adapterId,
                                                         make_shared<SetAdapterDirectProxyCallback>(shared_from_this(),
                                                                                                   response, exception,
                                                                                                   adapterId,
                                                                                                   replicaGroupId,
                                                                                                   proxy)))
                    {
                        return;
                    }
                }
            }

            adapter->setDirectProxyAsync(proxy, response, exception);
            return;
        }
        catch(const AdapterNotExistException&)
        {
            if(!_dynamicRegistration)
            {
                throw Ice::AdapterNotFoundException();
            }
        }
        catch(const Ice::Exception&)
        {
            auto traceLevels = _database->getTraceLevels();
            if(traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register adapter `" << adapterId << "' endpoints:\n" << toString(current_exception());
            }
            throw Ice::AdapterNotFoundException();
        }

        assert(_dynamicRegistration);
        if(_master)
        {
            try
            {
                _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                response();
                return;
            }
            catch(const AdapterExistsException&)
            {
                // Continue
            }
            catch(const DeploymentException& ex)
            {
                auto traceLevels = _database->getTraceLevels();
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
            auto session = _session.getSession();
            if(!session)
            {
                auto traceLevels = _database->getTraceLevels();
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
                response();
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
            catch(const Ice::LocalException&)
            {
                auto traceLevels = _database->getTraceLevels();
                if(traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter `" << adapterId << "' endpoints with master:\n"
                        << toString(current_exception());
                }
                throw Ice::AdapterNotFoundException();
            }
        }
    }
    while(nRetry-- > 0);
    throw Ice::AdapterNotFoundException();
}

const shared_ptr<TraceLevels>&
LocatorRegistryI::getTraceLevels() const
{
    return _database->getTraceLevels();
}
