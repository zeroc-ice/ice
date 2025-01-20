// Copyright (c) ZeroC, Inc.

#include "SessionServantManager.h"
#include "Ice/LocalExceptions.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/UUID.h"
#include <iterator>

using namespace std;
using namespace IceGrid;

SessionServantManager::SessionServantManager(
    Ice::ObjectAdapterPtr adapter,
    string instanceName,
    bool checkConnection,
    string serverAdminCategory,
    Ice::ObjectPtr serverAdminRouter,
    string nodeAdminCategory,
    Ice::ObjectPtr nodeAdminRouter,
    string replicaAdminCategory,
    Ice::ObjectPtr replicaAdminRouter,
    const shared_ptr<AdminCallbackRouter>& adminCallbackRouter)
    : _adapter(std::move(adapter)),
      _instanceName(std::move(instanceName)),
      _checkConnection(checkConnection),
      _serverAdminCategory(std::move(serverAdminCategory)),
      _serverAdminRouter(std::move(serverAdminRouter)),
      _nodeAdminCategory(std::move(nodeAdminCategory)),
      _nodeAdminRouter(std::move(nodeAdminRouter)),
      _replicaAdminCategory(std::move(replicaAdminCategory)),
      _replicaAdminRouter(std::move(replicaAdminRouter)),
      _adminCallbackRouter(adminCallbackRouter)
{
}

Ice::ObjectPtr
SessionServantManager::locate(const Ice::Current& current, shared_ptr<void>&)
{
    lock_guard lock(_mutex);

    Ice::ObjectPtr servant;
    bool plainServant = false;

    if (_serverAdminRouter && current.id.category == _serverAdminCategory)
    {
        servant = _serverAdminRouter;
    }
    else if (_nodeAdminRouter && current.id.category == _nodeAdminCategory)
    {
        servant = _nodeAdminRouter;
    }
    else if (_replicaAdminRouter && current.id.category == _replicaAdminCategory)
    {
        servant = _replicaAdminRouter;
    }
    else
    {
        plainServant = true;

        auto p = _servants.find(current.id);
        if (p == _servants.end() || (_checkConnection && p->second.connection != current.con))
        {
            servant = nullptr;
        }
        else
        {
            servant = p->second.servant;
        }
    }

    if (!plainServant && servant && _checkConnection && _adminConnections.find(current.con) == _adminConnections.end())
    {
        servant = nullptr;
    }

    return servant;
}

void
SessionServantManager::finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>&)
{
}

void
SessionServantManager::deactivate(string_view)
{
    lock_guard lock(_mutex);
    assert(_servants.empty());
    assert(_sessions.empty());
    assert(_adminConnections.empty());
}

Ice::ObjectPrx
SessionServantManager::addSession(
    const Ice::ObjectPtr& session,
    const shared_ptr<Ice::Connection>& connection,
    const string& category)
{
    lock_guard lock(_mutex);
    _sessions.insert({session, SessionInfo(connection, category)});

    //
    // Keep track of all the connections which have an admin session to allow access
    // to server admin objects.
    //
    if (!category.empty() && connection != nullptr)
    {
        _adminConnections.insert(connection);
        if (_adminCallbackRouter != nullptr)
        {
            _adminCallbackRouter->addMapping(category, connection);
        }
    }

    return addImpl(session, session); // Register a servant for the session and return its proxy.
}

void
SessionServantManager::setSessionControl(
    const Ice::ObjectPtr& session,
    const Glacier2::SessionControlPrx& ctl,
    const Ice::IdentitySeq& ids)
{
    lock_guard lock(_mutex);

    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    p->second.sessionControl = ctl;
    p->second.identitySet = ctl->identities();

    //
    // Allow invocations on the session servants and the given objects.
    //
    Ice::IdentitySeq allIds = ids;
    copy(p->second.identities.begin(), p->second.identities.end(), back_inserter(allIds));
    p->second.identitySet->add(allIds);

    // Allow invocations on server admin objects.
    if (!p->second.category.empty() && _serverAdminRouter)
    {
        ctl->categories()->add(Ice::StringSeq{_serverAdminCategory});
    }
}

optional<Glacier2::IdentitySetPrx>
SessionServantManager::getGlacier2IdentitySet(const Ice::ObjectPtr& session)
{
    lock_guard lock(_mutex);
    auto p = _sessions.find(session);
    if (p != _sessions.end() && p->second.sessionControl)
    {
        if (!p->second.identitySet) // Cache the identity set proxy
        {
            p->second.identitySet = p->second.sessionControl->identities();
        }
        return p->second.identitySet;
    }
    else
    {
        return nullopt;
    }
}

optional<Glacier2::StringSetPrx>
SessionServantManager::getGlacier2AdapterIdSet(const Ice::ObjectPtr& session)
{
    lock_guard lock(_mutex);
    auto p = _sessions.find(session);
    if (p != _sessions.end() && p->second.sessionControl)
    {
        if (!p->second.adapterIdSet) // Cache the adapterId set proxy
        {
            p->second.adapterIdSet = p->second.sessionControl->adapterIds();
        }
        return p->second.adapterIdSet;
    }
    else
    {
        return nullopt;
    }
}

void
SessionServantManager::removeSession(const Ice::ObjectPtr& session)
{
    lock_guard lock(_mutex);

    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    //
    // Remove all the servants associated with the session.
    //
    for (const auto& identity : p->second.identities)
    {
        _servants.erase(identity);
    }

    //
    // If this is an admin session, remove its connection from the admin connections.
    //

    if (!p->second.category.empty() && p->second.connection)
    {
        assert(_adminConnections.find(p->second.connection) != _adminConnections.end());
        _adminConnections.erase(_adminConnections.find(p->second.connection));

        if (_adminCallbackRouter != nullptr)
        {
            _adminCallbackRouter->removeMapping(p->second.category);
        }
    }

    _sessions.erase(p);
}

Ice::ObjectPrx
SessionServantManager::add(const Ice::ObjectPtr& servant, const Ice::ObjectPtr& session)
{
    lock_guard lock(_mutex);
    return addImpl(servant, session);
}

void
SessionServantManager::remove(const Ice::Identity& id)
{
    lock_guard lock(_mutex);
    auto p = _servants.find(id);
    assert(p != _servants.end());

    //
    // Find the session associated to the servant and remove the servant identity from the
    // session identities.
    //
    auto q = _sessions.find(p->second.session);
    assert(q != _sessions.end());
    q->second.identities.erase(id);

    //
    // Remove the identity from the Glacier2 identity set.
    //
    if (q->second.identitySet)
    {
        try
        {
            q->second.identitySet->remove({id});
        }
        catch (const Ice::LocalException&)
        {
        }
    }

    //
    // Remove the servant from the servant map.
    //
    _servants.erase(p);
}

Ice::ObjectPrx
SessionServantManager::addImpl(const Ice::ObjectPtr& servant, const Ice::ObjectPtr& session)
{
    auto p = _sessions.find(session);
    assert(p != _sessions.end());

    Ice::Identity id;
    id.name = Ice::generateUUID();
    id.category = _instanceName;

    //
    // Add the identity to the session identities.
    //
    p->second.identities.insert(id);

    //
    // Add the identity to the Glacier2 identity set.
    //
    if (p->second.identitySet)
    {
        try
        {
            p->second.identitySet->add({id});
        }
        catch (const Ice::LocalException&)
        {
        }
    }

    //
    // Add the servant to the servant map and return its proxy.
    //
    _servants.insert(make_pair(id, ServantInfo(servant, p->second.connection, session)));
    return _adapter->createProxy(id);
}
