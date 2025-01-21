// Copyright (c) ZeroC, Inc.

#include "RegistryAdminRouter.h"
#include "Ice/Ice.h"
#include "SynchronizationException.h"

using namespace IceGrid;
using namespace Ice;
using namespace std;

namespace
{
    class SynchronizationCallbackI final : public SynchronizationCallback
    {
    public:
        SynchronizationCallbackI(
            const shared_ptr<RegistryServerAdminRouter>& adminRouter,
            pair<const byte*, const byte*> inParams,
            function<void(bool, pair<const byte*, const byte*>)> response,
            function<void(exception_ptr)> exception,
            Current current)
            : _adminRouter(adminRouter),
              _response(std::move(response)),
              _exception(std::move(exception)),
              _inParams(inParams.first, inParams.second),
              _current(std::move(current))
        {
        }

        void synchronized() override
        {
            //
            // Retry to forward the call.
            //
            _adminRouter->ice_invokeAsync(
                {&_inParams[0], &_inParams[0] + _inParams.size()},
                std::move(_response),
                std::move(_exception),
                _current);
        }

        void synchronized(exception_ptr) override
        {
            _exception(make_exception_ptr(Ice::ObjectNotExistException{__FILE__, __LINE__}));
        }

    private:
        const shared_ptr<RegistryServerAdminRouter> _adminRouter;
        function<void(bool, pair<const byte*, const byte*>)> _response;
        function<void(exception_ptr)> _exception;
        const vector<byte> _inParams;
        const Current _current;
    };
}

RegistryServerAdminRouter::RegistryServerAdminRouter(const shared_ptr<Database>& database)
    : AdminRouter(database->getTraceLevels()),
      _database(database)
{
}

void
RegistryServerAdminRouter::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    optional<ObjectPrx> target;

    try
    {
        auto server = _database->getServer(current.id.name);
        try
        {
            target = server->getAdminProxy();
        }
        catch (const SynchronizationException&)
        {
            server->addSyncCallback(make_shared<SynchronizationCallbackI>(
                shared_from_this(),
                inParams,
                std::move(response),
                std::move(exception),
                current));
            return; // Wait for the server synchronization to complete and retry.
        }
    }
    catch (const ServerNotExistException&)
    {
    }
    catch (const NodeUnreachableException&)
    {
    }
    catch (const DeploymentException&)
    {
    }

    if (target == nullopt)
    {
        throw ObjectNotExistException{__FILE__, __LINE__};
    }
    invokeOnTarget(target->ice_facet(current.facet), inParams, std::move(response), std::move(exception), current);
}

RegistryNodeAdminRouter::RegistryNodeAdminRouter(string collocNodeName, const shared_ptr<Database>& database)
    : AdminRouter(database->getTraceLevels()),
      _collocNodeName(std::move(collocNodeName)),
      _database(database)
{
}

void
RegistryNodeAdminRouter::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    optional<ObjectPrx> target;

    if (!_collocNodeName.empty() && current.id.name == _collocNodeName)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            target = _database->getNode(current.id.name)->getAdminProxy();
        }
        catch (const NodeUnreachableException&)
        {
        }
        catch (const NodeNotExistException&)
        {
        }

        if (target == nullopt)
        {
            if (_traceLevels->admin > 0)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
                out << "could not find Admin proxy for node '" << current.id.name << "'";
            }

            throw ObjectNotExistException{__FILE__, __LINE__};
        }
    }

    invokeOnTarget(target->ice_facet(current.facet), inParams, std::move(response), std::move(exception), current);
}

RegistryReplicaAdminRouter::RegistryReplicaAdminRouter(string name, const shared_ptr<Database>& database)
    : AdminRouter(database->getTraceLevels()),
      _name(std::move(name)),
      _database(database)
{
}

void
RegistryReplicaAdminRouter::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> exception,
    const Ice::Current& current)
{
    optional<ObjectPrx> target;

    if (current.id.name == _name)
    {
        // Straight to the local Admin object
        target = current.adapter->getCommunicator()->getAdmin();
    }
    else
    {
        try
        {
            // Forward to Admin object in remote replica
            target = _database->getReplica(current.id.name)->getAdminProxy();
        }
        catch (const RegistryNotExistException&)
        {
        }
    }

    if (target == nullopt)
    {
        if (_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            out << "could not find Admin proxy for replica '" << current.id.name << "'";
        }

        throw ObjectNotExistException{__FILE__, __LINE__};
    }
    invokeOnTarget(target->ice_facet(current.facet), inParams, std::move(response), std::move(exception), current);
}
