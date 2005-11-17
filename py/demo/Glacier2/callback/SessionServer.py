#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, Glacier2

class DummyPermissionsVerifierI(Glacier2.PermissionsVerifier):
    def checkPermissions(self, userId, password, current=None):
        print "verified user `" + userId + "' with password `" + password + "'"
	return (True, "")

class SessionI(Glacier2.Session):
    def __init__(self, userId):
        self.userId = userId

    def destroy(self, current=None):
        print "destroying session for user `" + self.userId + "'"
	current.adapter.remove(current.id)

class SessionManagerI(Glacier2.SessionManager):
    def create(self, userId, current=None):
        print "creating session for user `" + userId + "'"
	session = SessionI(userId)
	return Glacier2.SessionPrx.uncheckedCast(current.adapter.addWithUUID(session))

class SessionServer(Ice.Application):
    def run(self, args):
	adapter = self.communicator().createObjectAdapter("SessionServer")
	adapter.add(DummyPermissionsVerifierI(), Ice.stringToIdentity("verifier"))
	adapter.add(SessionManagerI(), Ice.stringToIdentity("sessionmanager"))
	adapter.activate()
	self.communicator().waitForShutdown()
	return True

app = SessionServer()
sys.exit(app.main(sys.argv, "config.sessionserver"))
