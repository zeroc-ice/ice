//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_SESSIONSERVANTLOCATOR_H
#define ICE_GRID_SESSIONSERVANTLOCATOR_H

#include <IceGrid/AdminCallbackRouter.h>
#include <Ice/ServantLocator.h>

#include <Glacier2/Session.h>

#include <set>

namespace IceGrid
{

class SessionServantManager final : public Ice::ServantLocator
{
public:

    SessionServantManager(const std::shared_ptr<Ice::ObjectAdapter>&, const std::string&, bool,
                          const std::string&, const std::shared_ptr<Ice::Object>&,
                          const std::string&, const std::shared_ptr<Ice::Object>&,
                          const std::string&, const std::shared_ptr<Ice::Object>&,
                          const std::shared_ptr<AdminCallbackRouter>&);

    std::shared_ptr<Ice::Object> locate(const Ice::Current&, std::shared_ptr<void>&);
    void finished(const Ice::Current&, const std::shared_ptr<Ice::Object>&, const std::shared_ptr<void>&);
    void deactivate(const std::string&);

    std::shared_ptr<Ice::ObjectPrx> addSession(const std::shared_ptr<Ice::Object>&,
                                               const std::shared_ptr<Ice::Connection>&,
                                               const std::string&);
    void setSessionControl(const std::shared_ptr<Ice::Object>&,
                           const std::shared_ptr<Glacier2::SessionControlPrx>&, const Ice::IdentitySeq&);
    std::shared_ptr<Glacier2::IdentitySetPrx> getGlacier2IdentitySet(const std::shared_ptr<Ice::Object>&);
    std::shared_ptr<Glacier2::StringSetPrx> getGlacier2AdapterIdSet(const std::shared_ptr<Ice::Object>&);
    void removeSession(const std::shared_ptr<Ice::Object>&);

    std::shared_ptr<Ice::ObjectPrx> add(const std::shared_ptr<Ice::Object>&, const std::shared_ptr<Ice::Object>&);
    void remove(const Ice::Identity&);

    void connectionHeartbeat(const std::shared_ptr<Ice::Connection>&);
    void connectionClosed(const std::shared_ptr<Ice::Connection>&);

private:

    std::shared_ptr<Ice::ObjectPrx> addImpl(const std::shared_ptr<Ice::Object>&, const std::shared_ptr<Ice::Object>&);

    struct ServantInfo
    {
        ServantInfo(const std::shared_ptr<Ice::Object>& s,
                    const std::shared_ptr<Ice::Connection>& con,
                    const std::shared_ptr<Ice::Object>& ss) :
            servant(s), connection(con), session(ss)
        {
        }

        const std::shared_ptr<Ice::Object> servant;
        const std::shared_ptr<Ice::Connection> connection;
        const std::shared_ptr<Ice::Object> session;
    };

    struct SessionInfo
    {
        SessionInfo(const std::shared_ptr<Ice::Connection>& c, const std::string& cat) : connection(c), category(cat)
        {
        }

        const std::shared_ptr<Ice::Connection> connection;
        const std::string category;
        std::shared_ptr<Glacier2::SessionControlPrx> sessionControl;
        std::shared_ptr<Glacier2::IdentitySetPrx> identitySet;
        std::shared_ptr<Glacier2::StringSetPrx> adapterIdSet;
        std::set<Ice::Identity> identities;
    };

    const std::shared_ptr<Ice::ObjectAdapter> _adapter;
    const std::string _instanceName;
    const bool _checkConnection;
    const std::string _serverAdminCategory;
    const std::shared_ptr<Ice::Object> _serverAdminRouter;
    const std::string _nodeAdminCategory;
    const std::shared_ptr<Ice::Object> _nodeAdminRouter;
    const std::string _replicaAdminCategory;
    const std::shared_ptr<Ice::Object> _replicaAdminRouter;
    const std::shared_ptr<AdminCallbackRouter> _adminCallbackRouter;

    std::map<Ice::Identity, ServantInfo> _servants;
    std::map<std::shared_ptr<Ice::Object>, SessionInfo> _sessions;
    std::multiset<std::shared_ptr<Ice::Connection>> _adminConnections;

    std::mutex _mutex;

};

};

#endif
