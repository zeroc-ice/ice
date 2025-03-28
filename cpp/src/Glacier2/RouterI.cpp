// Copyright (c) ZeroC, Inc.

#include "RouterI.h"
#include "FilterManager.h"
#include "Glacier2/Session.h"
#include "RoutingTable.h"

#include <random>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RouterI::RouterI(
    shared_ptr<Instance> instance,
    ConnectionPtr connection,
    string userId,
    optional<SessionPrx> session,
    Identity controlId,
    shared_ptr<FilterManager> filters,
    const Context& context)
    : _instance(std::move(instance)),
      _routingTable(make_shared<RoutingTable>(_instance->communicator(), _instance->proxyVerifier())),
      _clientBlobject(make_shared<ClientBlobject>(_instance, std::move(filters), context, _routingTable)),
      _connection(std::move(connection)),
      _userId(std::move(userId)),
      _session(std::move(session)),
      _controlId(std::move(controlId)),
      _context(context)
{
    if (_instance->serverObjectAdapter())
    {
        Identity ident = {"dummy", ""};

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(33, 126); // We use ASCII 33-126 (from ! to ~, w/o space).
        for (unsigned int i = 0; i < 20; ++i)
        {
            ident.category.push_back(static_cast<char>(dist(gen)));
        }

        const_cast<optional<ObjectPrx>&>(_serverProxy) = _instance->serverObjectAdapter()->createProxy(ident);

        auto& serverBlobject = const_cast<shared_ptr<ServerBlobject>&>(_serverBlobject);
        serverBlobject = make_shared<ServerBlobject>(_instance, _connection);
    }

    if (_instance->getObserver())
    {
        updateObserver(_instance->getObserver());
    }
}

void
Glacier2::RouterI::destroy(function<void(exception_ptr)> error)
{
    if (_session)
    {
        if (_instance->serverObjectAdapter())
        {
            try
            {
                //
                // Remove the session control object.
                //
                _instance->serverObjectAdapter()->remove(_controlId);
            }
            catch (const NotRegisteredException&)
            {
            }
            catch (const ObjectAdapterDeactivatedException&)
            {
                //
                // Expected if the router has been shutdown.
                //
            }
        }

        if (_context.size() > 0)
        {
            _session->destroyAsync(nullptr, std::move(error), nullptr, _context);
        }
        else
        {
            _session->destroyAsync(nullptr, std::move(error), nullptr);
        }
    }

    _routingTable->destroy();
}

optional<ObjectPrx>
Glacier2::RouterI::getClientProxy(optional<bool>& hasRoutingTable, const Current&) const
{
    hasRoutingTable = true;
    return nullopt; // always return nullopt from Glacier2 router implementation
}

optional<ObjectPrx>
Glacier2::RouterI::getServerProxy(const Current&) const
{
    return _serverProxy;
}

ObjectProxySeq
Glacier2::RouterI::addProxies(ObjectProxySeq proxies, const Current& current)
{
    return _routingTable->add(std::move(proxies), current);
}

string
Glacier2::RouterI::getCategoryForClient(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return {};
}

void
Glacier2::RouterI::createSessionAsync(
    string,
    string,
    function<void(const optional<SessionPrx>& returnValue)>,
    function<void(exception_ptr)>,
    const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::createSessionFromSecureConnectionAsync(
    function<void(const optional<SessionPrx>& returnValue)>,
    function<void(exception_ptr)>,
    const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::destroySession(const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

int64_t
Glacier2::RouterI::getSessionTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

int32_t
Glacier2::RouterI::getACMTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

shared_ptr<ClientBlobject>
Glacier2::RouterI::getClientBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if (_observer)
    {
        _observer->forwarded(true);
    }
    return _clientBlobject;
}

shared_ptr<ServerBlobject>
Glacier2::RouterI::getServerBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if (_observer)
    {
        _observer->forwarded(false);
    }
    return _serverBlobject;
}

optional<SessionPrx>
Glacier2::RouterI::getSession() const
{
    return _session; // No mutex lock necessary, _session is immutable.
}

void
Glacier2::RouterI::updateObserver(const shared_ptr<Glacier2::Instrumentation::RouterObserver>& observer)
{
    // Can only be called with the SessionRouterI mutex locked

    _observer = _routingTable->updateObserver(observer, _userId, _connection);
}

string
Glacier2::RouterI::toString() const
{
    ostringstream out;

    out << "id = " << _userId << '\n';
    if (_serverProxy)
    {
        out << "category = " << _serverProxy->ice_getIdentity().category << '\n';
    }
    out << _connection->toString();

    return out.str();
}
