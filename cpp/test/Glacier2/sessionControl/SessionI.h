//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Session.h>

class SessionManagerI final : public Glacier2::SessionManager
{
public:
    Glacier2::SessionPrxPtr create(std::string, Glacier2::SessionControlPrxPtr, const Ice::Current&) override;
};

class SessionI final : public Test::Session
{
public:
    explicit SessionI(Glacier2::SessionControlPrxPtr);

    void destroyFromClientAsync(std::function<void()>,
                                std::function<void(std::exception_ptr)>,
                                const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
    void destroy(const Ice::Current&) override;

private:
    Glacier2::SessionControlPrxPtr _sessionControl;
};

#endif
