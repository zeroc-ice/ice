#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        status = 0
        registry = IceGrid.RegistryPrx.checkedCast(self.communicator().stringToProxy("DemoIceGrid/Registry"))
        if registry == None:
            print self.appName() + ": could not contact registry"
            return 1

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

        try:
            try:
                hello = Demo.HelloPrx.checkedCast(\
                    session.allocateObjectById(self.communicator().stringToIdentity("hello")))
            except IceGrid.ObjectNotRegisteredException:
                hello = Demo.HelloPrx.checkedCast(session.allocateObjectByType("::Demo::Hello"))

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
                except KeyboardInterrupt:
                    break
        except IceGrid.AllocationException, ex:
            print self.appName() + ": could not allocate object: " + ex.reason
            status = 1
        except:
            print self.appName() + ": could not allocate object: " + str(sys.exc_info()[0])
            status = 1

        #
        # Destroy the keepAlive thread and the sesion object otherwise
        # the session will be kept allocated until the timeout occurs.
        # Destroying the session will release all allocated objects.
        #
        keepAlive.terminate()
        keepAlive.join()
        session.destroy();

        return status

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
