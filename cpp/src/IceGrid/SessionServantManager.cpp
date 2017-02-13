// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/UUID.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <IceGrid/SessionServantManager.h>
#include <iterator>

using namespace std;
using namespace IceGrid;

SessionServantManager::SessionServantManager(const Ice::ObjectAdapterPtr& adapter,
                                             const string& instanceName,
                                             bool checkConnection,
                                             const string& serverAdminCategory,
                                             const Ice::ObjectPtr& serverAdminRouter,
                                             const string& nodeAdminCategory,
                                             const Ice::ObjectPtr& nodeAdminRouter,
                                             const string& replicaAdminCategory,
                                             const Ice::ObjectPtr& replicaAdminRouter,
                                             const AdminCallbackRouterPtr& adminCallbackRouter) :
    _adapter(adapter),
    _instanceName(instanceName),
    _checkConnection(checkConnection),
    _serverAdminCategory(serverAdminCategory),
    _serverAdminRouter(serverAdminRouter),
    _nodeAdminCategory(nodeAdminCategory),
    _nodeAdminRouter(nodeAdminRouter),
    _replicaAdminCategory(replicaAdminCategory),
    _replicaAdminRouter(replicaAdminRouter),
    _adminCallbackRouter(adminCallbackRouter)
{
}

Ice::ObjectPtr
SessionServantManager::locate(const Ice::Current& current, Ice::LocalObjectPtr&)
{
    Lock sync(*this);
    Ice::ObjectPtr servant;
    bool plainServant = false;

    if(_serverAdminRouter && current.id.category == _serverAdminCategory)
    {
        servant = _serverAdminRouter;
    }
    else if(_nodeAdminRouter && current.id.category == _nodeAdminCategory)
    {
        servant = _nodeAdminRouter;
    }
    else if(_replicaAdminRouter && current.id.category == _replicaAdminCategory)
    {
        servant = _replicaAdminRouter;
    }
    else
    {
        plainServant = true;

        map<Ice::Identity, ServantInfo>::const_iterator p = _servants.find(current.id);
        if(p == _servants.end() || (_checkConnection && p->second.connection != current.con))
        {
            servant = 0;
        }
        else
        {
            servant = p->second.servant;
        }
    }

    if(!plainServant && servant && _checkConnection &&
       _adminConnections.find(current.con) == _adminConnections.end())
    {
        servant = 0;
    }

    return servant;
}

void
SessionServantManager::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
{
}

void
SessionServantManager::deactivate(const std::string&)
{
    Lock sync(*this);
    assert(_servants.empty());
    assert(_sessions.empty());
    assert(_adminConnections.empty());
}

Ice::ObjectPrx
SessionServantManager::addSession(const Ice::ObjectPtr& session, const Ice::ConnectionPtr& con, const string& category)
{
    Lock sync(*this);
    _sessions.insert(make_pair(session, SessionInfo(con, category)));

    //
    // Keep track of all the connections which have an admin session to allow access
    // to server admin objects.
    //
    if(!category.empty() && con != 0)
    {
        _adminConnections.insert(con);
        if(_adminCallbackRouter != 0)
        {
            _adminCallbackRouter->addMapping(category, con);
        }
    }

    return addImpl(session, session); // Register a servant for the session and return its proxy.
}

void
SessionServantManager::setSessionControl(const Ice::ObjectPtr& session,
                                         const Glacier2::SessionControlPrx& ctl,
                                         const Ice::IdentitySeq& ids)
{
    Lock sync(*this);

    map<Ice::ObjectPtr, SessionInfo>::iterator p = _sessions.find(session);
    assert(p != _sessions.end());

    p->second.sessionControl = ctl;
    p->second.identitySet = ctl->identities();

    //
    // Allow invocations on the session servants and the given objects.
    //
    Ice::IdentitySeq allIds = ids;
    copy(p->second.identities.begin(), p->second.identities.end(), back_inserter(allIds));
    p->second.identitySet->add(allIds);

    //
    // Allow invocations on server admin objects.
    //
    if(!p->second.category.empty() && _serverAdminRouter)
    {
        Ice::StringSeq seq;
        seq.push_back(_serverAdminCategory);
        ctl->categories()->add(seq);
    }
}

Glacier2::IdentitySetPrx
SessionServantManager::getGlacier2IdentitySet(const Ice::ObjectPtr& session)
{
    Lock sync(*this);
    map<Ice::ObjectPtr, SessionInfo>::iterator p = _sessions.find(session);
    if(p != _sessions.end() && p->second.sessionControl)
    {
        if(!p->second.identitySet) // Cache the identity set proxy
        {
            p->second.identitySet = p->second.sessionControl->identities();
        }
        return p->second.identitySet;
    }
    else
    {
        return 0;
    }
}

Glacier2::StringSetPrx
SessionServantManager::getGlacier2AdapterIdSet(const Ice::ObjectPtr& session)
{
    Lock sync(*this);
    map<Ice::ObjectPtr, SessionInfo>::iterator p = _sessions.find(session);
    if(p != _sessions.end() && p->second.sessionControl)
    {
        if(!p->second.adapterIdSet) // Cache the adapterId set proxy
        {
            p->second.adapterIdSet = p->second.sessionControl->adapterIds();
        }
        return p->second.adapterIdSet;
    }
    else
    {
        return 0;
    }
}

void
SessionServantManager::removeSession(const Ice::ObjectPtr& session)
{
    Lock sync(*this);

    map<Ice::ObjectPtr, SessionInfo>::iterator p = _sessions.find(session);
    assert(p != _sessions.end());

    //
    // Remove all the servants associated with the session.
    //
    for(set<Ice::Identity>::const_iterator q = p->second.identities.begin(); q != p->second.identities.end(); ++q)
    {
        _servants.erase(*q);
    }

    //
    // If this is an admin session, remove its connection from the admin connections.
    //

    if(!p->second.category.empty() && p->second.connection)
    {
        assert(_adminConnections.find(p->second.connection) != _adminConnections.end());
        _adminConnections.erase(_adminConnections.find(p->second.connection));

        if(_adminCallbackRouter != 0)
        {
            _adminCallbackRouter->removeMapping(p->second.category);
        }
    }

    _sessions.erase(p);
}

Ice::ObjectPrx
SessionServantManager::add(const Ice::ObjectPtr& servant, const Ice::ObjectPtr& session)
{
    Lock sync(*this);
    return addImpl(servant, session);
}

void
SessionServantManager::remove(const Ice::Identity& id)
{
    Lock sync(*this);
    map<Ice::Identity, ServantInfo>::iterator p = _servants.find(id);
    assert(p != _servants.end());

    //
    // Find the session associated to the servant and remove the servant identity from the
    // session identities.
    //
    map<Ice::ObjectPtr, SessionInfo>::iterator q = _sessions.find(p->second.session);
    assert(q != _sessions.end());
    q->second.identities.erase(id);

    //
    // Remove the identity from the Glacier2 identity set.
    //
    if(q->second.identitySet)
    {
        try
        {
            Ice::IdentitySeq ids;
            ids.push_back(id);
            q->second.identitySet->remove(ids);
        }
        catch(const Ice::LocalException&)
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
    map<Ice::ObjectPtr, SessionInfo>::iterator p = _sessions.find(session);
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
    if(p->second.identitySet)
    {
        try
        {
            Ice::IdentitySeq ids;
            ids.push_back(id);
            p->second.identitySet->add(ids);
        }
        catch(const Ice::LocalException&)
        {
        }
    }

    //
    // Add the servant to the servant map and return its proxy.
    //
    _servants.insert(make_pair(id, ServantInfo(servant, p->second.connection, session)));
    return _adapter->createProxy(id);
}
