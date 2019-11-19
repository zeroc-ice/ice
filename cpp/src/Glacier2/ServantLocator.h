//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GLACIER2_LOCATOR_H
#define GLACIER2_LOCATOR_H

#include <Ice/Ice.h>

#include <Glacier2/SessionRouterI.h>

namespace Glacier2
{

class ClientLocator final : public Ice::ServantLocator
{
public:

    ClientLocator(std::shared_ptr<SessionRouterI> sessionRouter) :
        _sessionRouter(std::move(sessionRouter))
    {
    }

    std::shared_ptr<Ice::Object>
    locate(const Ice::Current& current, std::shared_ptr<void>&) override
    {
        return _sessionRouter->getClientBlobject(current.con, current.id);
    }

    void
    finished(const Ice::Current&, const std::shared_ptr<Ice::Object>&, const std::shared_ptr<void>&) override
    {
    }

    void
    deactivate(const std::string&) override
    {
    }

private:

    const std::shared_ptr<SessionRouterI> _sessionRouter;
};

class ServerLocator final : public Ice::ServantLocator
{
public:

    ServerLocator(std::shared_ptr<SessionRouterI> sessionRouter) :
        _sessionRouter(std::move(sessionRouter))
    {
    }

    std::shared_ptr<Ice::Object>
    locate(const Ice::Current& current, std::shared_ptr<void>&) override
    {
        return _sessionRouter->getServerBlobject(current.id.category);
    }

    void
    finished(const Ice::Current&, const std::shared_ptr<Ice::Object>&, const std::shared_ptr<void>&) override
    {
    }

    void
    deactivate(const std::string&) override
    {
    }

private:

    const std::shared_ptr<SessionRouterI> _sessionRouter;
};

}

#endif
