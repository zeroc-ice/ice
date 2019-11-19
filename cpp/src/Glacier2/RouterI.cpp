//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/FilterManager.h>
#include <Glacier2/RoutingTable.h>
#include <Glacier2/RouterI.h>
#include <Glacier2/Session.h>

#include<random>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::RouterI::RouterI(shared_ptr<Instance> instance, shared_ptr<Connection> connection,
                           const string& userId, shared_ptr<SessionPrx> session,
                           const Identity& controlId, shared_ptr<FilterManager> filters,
                           const Context& context) :
    _instance(move(instance)),
    _routingTable(make_shared<RoutingTable>(_instance->communicator(), _instance->proxyVerifier())),
    _clientBlobject(make_shared<ClientBlobject>(_instance, move(filters), context, _routingTable)),
    _clientBlobjectBuffered(_instance->clientRequestQueueThread()),
    _serverBlobjectBuffered(_instance->serverRequestQueueThread()),
    _connection(move(connection)),
    _userId(userId),
    _session(move(session)),
    _controlId(controlId),
    _context(context),
    _timestamp(chrono::steady_clock::now())
{
    //
    // If Glacier2 will be used with pre 3.2 clients, then the client proxy must be set.
    // Otherwise getClientProxy just needs to return a nil proxy.
    //
    if(_instance->properties()->getPropertyAsInt("Glacier2.ReturnClientProxy") > 0)
    {
        const_cast<shared_ptr<ObjectPrx>&>(_clientProxy) =
            _instance->clientObjectAdapter()->createProxy(stringToIdentity("dummy"));
    }

    if(_instance->serverObjectAdapter())
    {
        Identity ident = { "dummy", {} };

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dist(33, 126);  // We use ASCII 33-126 (from ! to ~, w/o space).
        for(unsigned int i = 0; i < 20; ++i)
        {
            ident.category.push_back(static_cast<char>(dist(gen)));
        }

        const_cast<shared_ptr<ObjectPrx>&>(_serverProxy) = _instance->serverObjectAdapter()->createProxy(ident);

        shared_ptr<ServerBlobject>& serverBlobject = const_cast<shared_ptr<ServerBlobject>&>(_serverBlobject);
        serverBlobject = make_shared<ServerBlobject>(_instance, _connection);
    }

    if(_instance->getObserver())
    {
        updateObserver(_instance->getObserver());
    }
}

Glacier2::RouterI::~RouterI()
{
}

void
Glacier2::RouterI::destroy(function<void(exception_ptr)> error)
{
    if(_session)
    {
        if(_instance->serverObjectAdapter())
        {
            try
            {
                //
                // Remove the session control object.
                //
                _instance->serverObjectAdapter()->remove(_controlId);
            }
            catch(const NotRegisteredException&)
            {
            }
            catch(const ObjectAdapterDeactivatedException&)
            {
                //
                // Expected if the router has been shutdown.
                //
            }
        }

        if(_context.size() > 0)
        {
            _session->destroyAsync(nullptr, move(error), nullptr, _context);
        }
        else
        {
            _session->destroyAsync(nullptr, move(error), nullptr);
        }
    }

    _clientBlobject->destroy();
    if(_serverBlobject)
    {
        _serverBlobject->destroy();
    }

    _routingTable->destroy();
}

shared_ptr<ObjectPrx>
Glacier2::RouterI::getClientProxy(Ice::optional<bool>& hasRoutingTable, const Current&) const
{
    // No mutex lock necessary, _clientProxy is immutable and is never destroyed.
    hasRoutingTable = true;
    return _clientProxy;
}

shared_ptr<ObjectPrx>
Glacier2::RouterI::getServerProxy(const Current&) const
{
    // No mutex lock necessary, _serverProxy is immutable and is never destroyed.
    return _serverProxy;
}

ObjectProxySeq
Glacier2::RouterI::addProxies(ObjectProxySeq proxies, const Current& current)
{
    return _routingTable->add(move(proxies), current);
}

string
Glacier2::RouterI::getCategoryForClient(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

void
Glacier2::RouterI::createSessionAsync(string, string,
                            function<void(const shared_ptr<SessionPrx>& returnValue)>,
                            function<void(exception_ptr)>, const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::createSessionFromSecureConnectionAsync(function<void(const shared_ptr<SessionPrx>& returnValue)>,
                                                          function<void(exception_ptr)>, const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::refreshSessionAsync(function<void()>, function<void(exception_ptr)>, const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

void
Glacier2::RouterI::destroySession(const Current&)
{
    assert(false); // Must not be called in this router implementation.
}

Long
Glacier2::RouterI::getSessionTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

Int
Glacier2::RouterI::getACMTimeout(const Current&) const
{
    assert(false); // Must not be called in this router implementation.
    return 0;
}

shared_ptr<ClientBlobject>
Glacier2::RouterI::getClientBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if(!_clientBlobjectBuffered && _observer)
    {
        _observer->forwarded(true);
    }
    return _clientBlobject;
}

shared_ptr<ServerBlobject>
Glacier2::RouterI::getServerBlobject() const
{
    // Can only be called with the SessionRouterI mutex locked
    if(!_serverBlobjectBuffered && _observer)
    {
        _observer->forwarded(false);
    }
    return _serverBlobject;
}

shared_ptr<SessionPrx>
Glacier2::RouterI::getSession() const
{
    return _session; // No mutex lock necessary, _session is immutable.
}

chrono::steady_clock::time_point
Glacier2::RouterI::getTimestamp() const
{
    // Can only be called with the SessionRouterI mutex locked
    return _timestamp;
}

void
Glacier2::RouterI::updateTimestamp() const
{
    // Can only be called with the SessionRouterI mutex locked
    _timestamp = chrono::steady_clock::now();
}

void
Glacier2::RouterI::updateObserver(const shared_ptr<Glacier2::Instrumentation::RouterObserver>& observer)
{
    // Can only be called with the SessionRouterI mutex locked

    _observer = _routingTable->updateObserver(observer, _userId, _connection);
    _clientBlobject->updateObserver(_observer);
    if(_serverBlobject)
    {
        _serverBlobject->updateObserver(_observer);
    }
}

string
Glacier2::RouterI::toString() const
{
    ostringstream out;

    out << "id = " << _userId << '\n';
    if(_serverProxy)
    {
        out << "category = " << _serverProxy->ice_getIdentity().category << '\n';
    }
    out << _connection->toString();

    return out.str();
}
