// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Session.h>

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);
};


class SessionI : public Test::Session
{
public:

    SessionI(const Glacier2::SessionControlPrx&);

    virtual void destroyFromClient_async(const Test::AMD_Session_destroyFromClientPtr&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    virtual void destroy(const Ice::Current&);

private:

    Glacier2::SessionControlPrx _sessionControl;
};

#endif
