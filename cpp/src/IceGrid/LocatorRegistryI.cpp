// Copyright (c) ZeroC, Inc.

#include "LocatorRegistryI.h"
#include "Database.h"
#include "Ice/Ice.h"
#include "ReplicaSessionManager.h"
#include "SynchronizationException.h"
#include "Util.h"

using namespace std;
using namespace IceGrid;

namespace IceGrid
{
    tuple<function<void()>, function<void(exception_ptr)>> newSetDirectProxyCB(
        function<void()> responseCb,
        function<void(exception_ptr)> exceptionCb,
        const shared_ptr<TraceLevels>& traceLevels,
        const string& id,
        const optional<Ice::ObjectPrx>& proxy)
    {
        auto response = [traceLevels, id, proxy, responseCb = std::move(responseCb)]()
        {
            if (traceLevels->locator > 1)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "registered adapter '" << id << "' endpoints: '";
                out << (proxy ? proxy->ice_toString() : string("")) << "'";
            }
            responseCb();
        };

        auto exception = [traceLevels, id, exceptionCb = std::move(exceptionCb)](auto exptr)
        {
            if (traceLevels->locator > 1)
            {
                try
                {
                    rethrow_exception(exptr);
                }
                catch (const std::exception& ex)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "failed to register adapter '" << id << "' endpoints:\n" << ex;
                }
            }

            try
            {
                rethrow_exception(exptr);
            }
            catch (const AdapterActiveException&)
            {
                exceptionCb(make_exception_ptr(Ice::AdapterAlreadyActiveException()));
            }
            catch (const Ice::ObjectNotExistException&)
            {
                exceptionCb(
                    make_exception_ptr(Ice::AdapterNotFoundException())); // Expected if the adapter was destroyed).
            }
            catch (const Ice::Exception&)
            {
                exceptionCb(make_exception_ptr(Ice::AdapterNotFoundException()));
            }
        };

        return {std::move(response), std::move(exception)};
    }

    class SetAdapterDirectProxyCallback final : public SynchronizationCallback
    {
    public:
        SetAdapterDirectProxyCallback(
            const shared_ptr<LocatorRegistryI>& registry,
            function<void()> response,
            function<void(exception_ptr)> exception,
            string adapterId,
            string replicaGroupId,
            optional<Ice::ObjectPrx> proxy)
            : _registry(registry),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _adapterId(std::move(adapterId)),
              _replicaGroupId(std::move(replicaGroupId)),
              _proxy(std::move(proxy))
        {
        }

        void synchronized() override
        {
            try
            {
                _registry->setAdapterDirectProxy(_adapterId, _replicaGroupId, _proxy, _response, _exception);
            }
            catch (const Ice::Exception&)
            {
                _exception(current_exception());
            }
        }

        void synchronized(exception_ptr ex) override { _exception(ex); }

    private:
        const shared_ptr<LocatorRegistryI> _registry;
        const function<void()> _response;
        const function<void(exception_ptr)> _exception;
        const string _adapterId;
        const string _replicaGroupId;
        const optional<Ice::ObjectPrx> _proxy;
    };

    class SetServerProcessProxyCallback final : public SynchronizationCallback
    {
    public:
        SetServerProcessProxyCallback(
            shared_ptr<LocatorRegistryI> registry,
            function<void()> response,
            function<void(exception_ptr)> exception,
            string id,
            Ice::ProcessPrx proxy)
            : _registry(std::move(registry)),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _id(std::move(id)),
              _proxy(std::move(proxy))
        {
        }

        void synchronized() override
        {
            try
            {
                _registry->setServerProcessProxyAsync(_id, _proxy, _response, _exception, Ice::Current());
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
            catch (const ServerNotExistException&)
            {
                _exception(make_exception_ptr(Ice::ServerNotFoundException()));
            }
            catch (const Ice::Exception&)
            {
                auto traceLevels = _registry->getTraceLevels();
                if (traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register server '" << _id << "' process proxy:\n" << toString(current_exception());
                }
                _exception(make_exception_ptr(Ice::ServerNotFoundException()));
            }
        }

    private:
        const shared_ptr<LocatorRegistryI> _registry;
        const function<void()> _response;
        const function<void(exception_ptr)> _exception;
        const string _id;
        const Ice::ProcessPrx _proxy;
    };
};

LocatorRegistryI::LocatorRegistryI(
    const shared_ptr<Database>& database,
    bool dynamicRegistration,
    bool master,
    ReplicaSessionManager& session)
    : _database(database),
      _dynamicRegistration(dynamicRegistration),
      _master(master),
      _session(session)
{
}

void
LocatorRegistryI::setAdapterDirectProxyAsync(
    string adapterId,
    optional<Ice::ObjectPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    auto [responseCb, exceptionCb] =
        newSetDirectProxyCB(std::move(response), std::move(exception), _database->getTraceLevels(), adapterId, proxy);

    setAdapterDirectProxy(adapterId, "", proxy, std::move(responseCb), std::move(exceptionCb));
}

void
LocatorRegistryI::setReplicatedAdapterDirectProxyAsync(
    string adapterId,
    string replicaGroupId,
    optional<Ice::ObjectPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current&)
{
    auto [responseCb, exceptionCb] =
        newSetDirectProxyCB(std::move(response), std::move(exception), _database->getTraceLevels(), adapterId, proxy);
    setAdapterDirectProxy(adapterId, replicaGroupId, proxy, std::move(responseCb), std::move(exceptionCb));
}

void
LocatorRegistryI::setServerProcessProxyAsync(
    string id,
    optional<Ice::ProcessPrx> proxy,
    function<void()> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    try
    {
        Ice::checkNotNull(proxy, __FILE__, __LINE__, current);

        // Get the server from the registry and set its process proxy.
        //
        // NOTE: We pass false to the getServer call to indicate that we don't necessarily want an up-to-date adapter
        // proxy. This is needed for the session activation mode for cases where the server is released during the
        // server startup.
        optional<ServerPrx> server;
        while (true)
        {
            try
            {
                server = _database->getServer(id)->getProxy(false);
                break;
            }
            catch (const SynchronizationException&)
            {
                if (_database->getServer(id)->addSyncCallback(make_shared<SetServerProcessProxyCallback>(
                        shared_from_this(),
                        response,
                        exception,
                        id,
                        *proxy)))
                {
                    return;
                }
            }
        }

        server->setProcessAsync(
            proxy,
            [id, proxy, response, traceLevels = _database->getTraceLevels()]
            {
                if (traceLevels->locator > 1)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "registered server '" << id << "' process proxy: '" << proxy->ice_toString() << "'";
                }
                response();
            },
            [id, exception, traceLevels = _database->getTraceLevels()](exception_ptr exptr)
            {
                if (traceLevels->locator > 1)
                {
                    try
                    {
                        rethrow_exception(exptr);
                    }
                    catch (const std::exception& ex)
                    {
                        Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                        out << "failed to register server process proxy '" << id << "':\n" << ex;
                    }
                }

                try
                {
                    rethrow_exception(exptr);
                }
                catch (const Ice::ObjectNotExistException&)
                {
                    // Expected if the server was destroyed.
                    exception(make_exception_ptr(Ice::ServerNotFoundException()));
                    return;
                }
                catch (const Ice::LocalException&)
                {
                    exception(make_exception_ptr(Ice::ServerNotFoundException()));
                    return;
                }
            });
    }
    catch (const ServerNotExistException&)
    {
        exception(make_exception_ptr(Ice::ServerNotFoundException()));
    }
    catch (const Ice::Exception&)
    {
        auto traceLevels = _database->getTraceLevels();
        if (traceLevels->locator > 0)
        {
            Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
            out << "couldn't register server '" << id << "' process proxy:\n" << toString(current_exception());
        }
        exception(make_exception_ptr(Ice::ServerNotFoundException()));
    }
}

void
LocatorRegistryI::setAdapterDirectProxy(
    string adapterId,                        // NOLINT(performance-unnecessary-value-param)
    string replicaGroupId,                   // NOLINT(performance-unnecessary-value-param)
    optional<Ice::ObjectPrx> proxy,          // NOLINT(performance-unnecessary-value-param)
    function<void()> response,               // NOLINT(performance-unnecessary-value-param)
    function<void(exception_ptr)> exception) // NOLINT(performance-unnecessary-value-param)
{
    // Ignore request with empty adapter id.
    if (adapterId.empty())
    {
        response();
        return;
    }

    int nRetry = 5;
    do
    {
        try
        {
            // Get the adapter from the registry and set its direct proxy.
            optional<AdapterPrx> adapter;
            while (true)
            {
                try
                {
                    adapter = _database->getAdapterProxy(adapterId, replicaGroupId, false);
                    if (!adapter)
                    {
                        throw Ice::AdapterNotFoundException();
                    }
                    break;
                }
                catch (const SynchronizationException&)
                {
                    if (_database->addAdapterSyncCallback(
                            adapterId,
                            make_shared<SetAdapterDirectProxyCallback>(
                                shared_from_this(),
                                response,
                                exception,
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
        catch (const AdapterNotExistException&)
        {
            if (!_dynamicRegistration)
            {
                throw Ice::AdapterNotFoundException();
            }
        }
        catch (const Ice::Exception&)
        {
            auto traceLevels = _database->getTraceLevels();
            if (traceLevels->locator > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                out << "couldn't register adapter '" << adapterId << "' endpoints:\n" << toString(current_exception());
            }
            throw Ice::AdapterNotFoundException();
        }

        assert(_dynamicRegistration);
        if (_master)
        {
            try
            {
                _database->setAdapterDirectProxy(adapterId, replicaGroupId, proxy);
                response();
                return;
            }
            catch (const AdapterExistsException&)
            {
                // Continue
            }
            catch (const DeploymentException& ex)
            {
                auto traceLevels = _database->getTraceLevels();
                if (traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter '" << adapterId << "' endpoints with master:\n" << ex.reason;
                }
                throw Ice::AdapterNotFoundException();
            }
        }
        else
        {
            auto session = _session.getSession();
            if (!session)
            {
                auto traceLevels = _database->getTraceLevels();
                if (traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter '" << adapterId << "' endpoints with master:\n";
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
            catch (const AdapterExistsException&)
            {
                // Continue
            }
            catch (const AdapterNotExistException&)
            {
                throw Ice::AdapterNotFoundException(); // Dynamic registration not allowed on the master.
            }
            catch (const Ice::LocalException&)
            {
                auto traceLevels = _database->getTraceLevels();
                if (traceLevels->locator > 0)
                {
                    Ice::Trace out(traceLevels->logger, traceLevels->locatorCat);
                    out << "couldn't register adapter '" << adapterId << "' endpoints with master:\n"
                        << toString(current_exception());
                }
                throw Ice::AdapterNotFoundException();
            }
        }
    } while (nRetry-- > 0);
    throw Ice::AdapterNotFoundException();
}

const shared_ptr<TraceLevels>&
LocatorRegistryI::getTraceLevels() const
{
    return _database->getTraceLevels();
}
