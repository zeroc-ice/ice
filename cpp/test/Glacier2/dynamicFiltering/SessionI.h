//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Test.h>
#include <TestControllerI.h>

// XXX

//
// The session manager essentially has to be the test controller since
// it will be the entity that is notified of session creation. Can it be
// notified of session destruction as well? There really isn't any
// reason why not as long as there isn't a deadlock situation.
//

class SessionManagerI final : public Glacier2::SessionManager
{
public:
    SessionManagerI(const std::shared_ptr<TestControllerI>&);

    Glacier2::SessionPrxPtr
    create(std::string, Glacier2::SessionControlPrxPtr, const Ice::Current&) override;

private:
    std::shared_ptr<TestControllerI> _controller;
};

class SessionI final : public Test::TestSession
{
public:

    SessionI(Glacier2::SessionControlPrxPtr, std::shared_ptr<TestControllerI>);
    void shutdown(const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

private:

    Glacier2::SessionControlPrxPtr _sessionControl;
    std::shared_ptr<TestControllerI> _controller;
};

#endif
