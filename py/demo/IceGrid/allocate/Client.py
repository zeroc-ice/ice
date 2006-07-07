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
t: send greeting
s: shutdown server
x: exit
?: help
"""

class SessionKeepAliveThread(threading.Thread):
    def __init__(self, session, timeout):
        threading.Thread.__init__(self)
        self._session = session
        self._timeout = timeout
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
	registry = IceGrid.RegistryPrx.checkedCast(self.communicator().stringToProxy("DemoIceGrid/Registry"))
	if registry == None:
	    print self.appName() + ": could not contact registry"
	    return False

	while True:
	    print "This demo accepts any user-id / password combination."
	    id = raw_input("user id: ").strip()
	    pw = raw_input("password: ").strip()
	    try:
	        session = registry.createSession(id, pw)
		break
	    except IceGrid.PermissionDeniedException, ex:
	        print "permission denied:\n" + ex.reason

	keepAlive = SessionKeepAliveThread(session, registry.getSessionTimeout() / 2)
	keepAlive.start()

        hello = None
	try:
	    hello = Demo.HelloPrx.checkedCast(session.allocateObjectById(self.communicator().stringToIdentity("hello")))
	except IceGrid.AllocationException, ex:
            print self.appName() + ": could not allocate object: " + ex.reason
            return False
	except IceGrid.ObjectNotRegisteredException:
	    pass
	if hello == None:
	    try:
	        hello = Demo.HelloPrx.checkedCast(session.allocateObjectByType("::Demo::Hello"))
	    except IceGrid.AllocationException, ex:
	        print self.appName() + ": could not allocate object: " + ex.reason
	        return False

	menu()

	c = None
	while c != 'x':
	    try:
		c = raw_input("==> ")
		if c == 't':
		    hello.sayHello()
		elif c == 's':
		    hello.shutdown()
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
	session.destroy();

	return True

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
