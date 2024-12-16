//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_SESSIONSERVANTLOCATOR_H
#define ICEGRID_SESSIONSERVANTLOCATOR_H

#include "AdminCallbackRouter.h"
#include "Ice/ServantLocator.h"

#include "Glacier2/Session.h"

#include <set>

namespace IceGrid
{
    class SessionServantManager final : public Ice::ServantLocator
    {
    public:
        SessionServantManager(
            const Ice::ObjectAdapterPtr&,
            const std::string&,
            bool,
            const std::string&,
            const Ice::ObjectPtr&,
            const std::string&,
            const Ice::ObjectPtr&,
            const std::string&,
            const Ice::ObjectPtr&,
            const std::shared_ptr<AdminCallbackRouter>&);

        Ice::ObjectPtr locate(const Ice::Current&, std::shared_ptr<void>&) override;
        void finished(const Ice::Current&, const Ice::ObjectPtr&, const std::shared_ptr<void>&) override;
        void deactivate(std::string_view) override;

        Ice::ObjectPrx addSession(const Ice::ObjectPtr&, const Ice::ConnectionPtr&, const std::string&);
        void setSessionControl(const Ice::ObjectPtr&, const Glacier2::SessionControlPrx&, const Ice::IdentitySeq&);
        std::optional<Glacier2::IdentitySetPrx> getGlacier2IdentitySet(const Ice::ObjectPtr&);
        std::optional<Glacier2::StringSetPrx> getGlacier2AdapterIdSet(const Ice::ObjectPtr&);
        void removeSession(const Ice::ObjectPtr&);

        Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::ObjectPtr&);
        void remove(const Ice::Identity&);

        void connectionClosed(const Ice::ConnectionPtr&);

    private:
        Ice::ObjectPrx addImpl(const Ice::ObjectPtr&, const Ice::ObjectPtr&);

        struct ServantInfo
        {
            ServantInfo(const Ice::ObjectPtr& s, const Ice::ConnectionPtr& con, const Ice::ObjectPtr& ss)
                : servant(s),
                  connection(con),
                  session(ss)
            {
            }

            const Ice::ObjectPtr servant;
            const Ice::ConnectionPtr connection;
            const Ice::ObjectPtr session;
        };

        struct SessionInfo
        {
            SessionInfo(const Ice::ConnectionPtr& c, const std::string& cat) : connection(c), category(cat) {}

            const Ice::ConnectionPtr connection;
            const std::string category;
            std::optional<Glacier2::SessionControlPrx> sessionControl;
            std::optional<Glacier2::IdentitySetPrx> identitySet;
            std::optional<Glacier2::StringSetPrx> adapterIdSet;
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
        const std::shared_ptr<AdminCallbackRouter> _adminCallbackRouter;

        std::map<Ice::Identity, ServantInfo> _servants;
        std::map<Ice::ObjectPtr, SessionInfo> _sessions;
        std::multiset<Ice::ConnectionPtr> _adminConnections;

        std::mutex _mutex;
    };

};

#endif
