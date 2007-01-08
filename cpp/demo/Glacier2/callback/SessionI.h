// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SESSION_I_H
#define SESSION_I_H

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>

class DummyPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool checkPermissions(const std::string&, const std::string&, std::string&, const Ice::Current&) const;
};

class SessionI : public Glacier2::Session
{
public:

    SessionI(const std::string&);

    virtual void destroy(const Ice::Current&);

private:

    const std::string _userId;
};

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx create(const std::string&, const Glacier2::SessionControlPrx&, const Ice::Current&);
};

#endif
