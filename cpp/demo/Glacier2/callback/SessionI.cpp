// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <SessionI.h>

using namespace std;

bool
DummyPermissionsVerifierI::checkPermissions(const string& userId, const string& password, string&,
                                            const Ice::Current&) const
{
    cout << "verified user `" << userId << "' with password `" << password << "'" << endl;
    return true;
}

SessionI::SessionI(const string& userId) :
    _userId(userId)
{
}

void
SessionI::destroy(const Ice::Current& current)
{
    cout << "destroying session for user `" << _userId << "'" << endl;
    current.adapter->remove(current.id);
}

Glacier2::SessionPrx
SessionManagerI::create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
{
    cout << "creating session for user `" << userId << "'" << endl;
    Glacier2::SessionPtr session = new SessionI(userId);
    return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
}
