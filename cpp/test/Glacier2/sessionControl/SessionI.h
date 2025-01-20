// Copyright (c) ZeroC, Inc.

#ifndef SESSION_I_H
#define SESSION_I_H

#include "Session.h"

class SessionManagerI final : public Glacier2::SessionManager
{
public:
    std::optional<Glacier2::SessionPrx>
    create(std::string, std::optional<Glacier2::SessionControlPrx>, const Ice::Current&) override;
};

class SessionI final : public Test::Session
{
public:
    explicit SessionI(std::optional<Glacier2::SessionControlPrx>);

    void destroyFromClientAsync(std::function<void()>, std::function<void(std::exception_ptr)>, const Ice::Current&)
        override;

    void shutdown(const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

private:
    std::optional<Glacier2::SessionControlPrx> _sessionControl;
};

#endif
