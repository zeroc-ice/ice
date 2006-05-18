#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, threading, Ice, IceGrid

Ice.loadSlice('Hello.ice')
import Demo

def menu():
    print """
usage:
t: send greeting as twoway
x: exit
?: help
"""

class SessionKeepAliveThread(threading.Thread):
    def __init__(self, session):
        threading.Thread.__init__(self)
        self._session = session
        self._timeout = 5
        self._terminated = False
        self._cond = threading.Condition()

    def run(self):
        self._cond.acquire()
        try:
            while not self._terminated:
                self._cond.wait(self._timeout)
                if self._terminated:
		    break
                try:
                    self._session.keepAlive()
                except Ice.LocalException, ex:
		    break
        finally:
            self._cond.release()

    def terminate(self):
        self._cond.acquire()
        try:
            self._terminated = True
            self._cond.notify()
        finally:
            self._cond.release()

class Client(Ice.Application):
    def run(self, args):
	proxyStr = "DemoIceGrid/SessionManager"
	sessionManager = IceGrid.SessionManagerPrx.checkedCast(self.communicator().stringToProxy(proxyStr))
	if sessionManager == None:
	    print self.appName() + ": cound not contact session manager"
	    return False

	while True:
	    id = raw_input("user id: ").strip()
	    if len(id) != 0:
	        break

	session = sessionManager.createLocalSession(id)

	keepAlive = SessionKeepAliveThread(session)
	keepAlive.start()

        hello = None
	try:
	    hello = Demo.HelloPrx.checkedCast(session.allocateObjectById(self.communicator().stringToIdentity("hello")))
	except IceGrid.AllocationException, ex:
	    print self.appName() + ": could not allocate object: " + ex.reason
	    return False
	except IceGrid.ObjectNotRegisteredException:
	    print self.appName() + ": object not registered with registry"
	    return False

	menu()

	c = None
	while c != 'x':
	    try:
		c = raw_input("==> ")
		if c == 't':
		    hello.sayHello()
		elif c == 'x':
		    pass # Nothing to do
		elif c == '?':
		    menu()
		else:
		    print "unknown command `" + c + "'"
		    menu()
	    except EOFError:
		break

	keepAlive.terminate()
	keepAlive.join()

	session.releaseObject(hello.ice_getIdentity())

	return True

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
