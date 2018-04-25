// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_SESSIONSERVANTLOCATOR_H
#define ICE_GRID_SESSIONSERVANTLOCATOR_H

#include <IceGrid/AdminCallbackRouter.h>
#include <IceUtil/Mutex.h>
#include <Ice/ServantLocator.h>

#include <Glacier2/Session.h>

#include <set>

namespace IceGrid
{

class SessionServantManager : public Ice::ServantLocator, public IceUtil::Mutex
{
public:

    SessionServantManager(const Ice::ObjectAdapterPtr&, const std::string&, bool,
                          const std::string&, const Ice::ObjectPtr&,
                          const std::string&, const Ice::ObjectPtr&,
                          const std::string&, const Ice::ObjectPtr&,
                          const AdminCallbackRouterPtr&);

    Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    void deactivate(const std::string&);

    Ice::ObjectPrx addSession(const Ice::ObjectPtr&, const Ice::ConnectionPtr&, const std::string&);
    void setSessionControl(const Ice::ObjectPtr&, const Glacier2::SessionControlPrx&, const Ice::IdentitySeq&);
    Glacier2::IdentitySetPrx getGlacier2IdentitySet(const Ice::ObjectPtr&);
    Glacier2::StringSetPrx getGlacier2AdapterIdSet(const Ice::ObjectPtr&);
    void removeSession(const Ice::ObjectPtr&);

    Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::ObjectPtr&);
    void remove(const Ice::Identity&);

    void connectionHeartbeat(const Ice::ConnectionPtr&);
    void connectionClosed(const Ice::ConnectionPtr&);

private:

    Ice::ObjectPrx addImpl(const Ice::ObjectPtr&, const Ice::ObjectPtr&);

    struct ServantInfo
    {
        ServantInfo(const Ice::ObjectPtr& s, const Ice::ConnectionPtr& con, const Ice::ObjectPtr& ss) :
            servant(s), connection(con), session(ss)
        {
        }

        const Ice::ObjectPtr servant;
        const Ice::ConnectionPtr connection;
        const Ice::ObjectPtr session;
    };

    struct SessionInfo
    {
        SessionInfo(const Ice::ConnectionPtr& c, const std::string& cat) : connection(c), category(cat) { }

        const Ice::ConnectionPtr connection;
        const std::string category;
        Glacier2::SessionControlPrx sessionControl;
        Glacier2::IdentitySetPrx identitySet;
        Glacier2::StringSetPrx adapterIdSet;
        std::set<Ice::Identity> identities;
    };

    const Ice::ObjectAdapterPtr _adapter;
    const std::string _instanceName;
    const bool _checkConnection;
    const std::string _serverAdminCategory;
    const Ice::ObjectPtr _serverAdminRouter;
    const std::string _nodeAdminCategory;
    const Ice::ObjectPtr _nodeAdminRouter;
    const std::string _replicaAdminCategory;
    const Ice::ObjectPtr _replicaAdminRouter;
    const AdminCallbackRouterPtr _adminCallbackRouter;

    std::map<Ice::Identity, ServantInfo> _servants;
    std::map<Ice::ObjectPtr, SessionInfo> _sessions;
    std::multiset<Ice::ConnectionPtr> _adminConnections;

};
typedef IceUtil::Handle<SessionServantManager> SessionServantManagerPtr;

};

#endif
