#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, threading, Ice

Ice.loadSlice('Session.ice')
import Demo

class HelloI(Demo.Hello):
    def __init__(self, name, id):
        self._name = name
        self._id = id

    def sayHello(self, c):
        print "Hello object #" + str(self._id) + " for session `" + self._name + "' says:\n" + \
              "Hello " + self._name + "!"

class SessionI(Demo.Session):
    def __init__(self, name):
        self._timestamp = time.time()
        self._name = name
        self._lock = threading.Lock()
        self._destroy = False # true if destroy() was called, false otherwise.
        self._nextId = 0 # The id of the next hello object. This is used for tracing purposes.
        self._objs = [] # List of per-client allocated Hello objects.

        print  "The session " + self._name + " is now created."
    
    def createHello(self, c):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()

            hello = Demo.HelloPrx.uncheckedCast(c.adapter.addWithUUID(HelloI(self._name, self._nextId)))
            self._nextId = self._nextId + 1
            self._objs.append(hello)
            return hello
        finally:
            self._lock.release()

    def refresh(self, c):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            self._timestamp = time.time()
        finally:
            self._lock.release()

    def getName(self, c):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            return self._name
        finally:
            self._lock.release()
    
    def destroy(self, c):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            self._destroy = True
            print "The session " + self._name + " is now destroyed."
            try:
                c.adapter.remove(c.id)
                for p in self._objs:
                    c.adapter.remove(p.ice_getIdentity())
            except Ice.ObjectAdapterDeactivatedException, ex:
                # This method is called on shutdown of the server, in
                # which case this exception is expected.
                pass
            self._objs = []
        finally:
            self._lock.release()

    def timestamp(self):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            return self._timestamp
        finally:
            self._lock.release()
        
class SessionProxyPair:
    def __init__(self, p, s):
        self.proxy = p
        self.session = s

class ReapThread(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self._timeout = 10
        self._terminated = False
        self._cond = threading.Condition()
        self._sessions = []

    def run(self):
        self._cond.acquire()
        try:
            while not self._terminated:
                self._cond.wait(self._timeout)
                if not self._terminated:
                    for p in self._sessions:
                        try:
                            #
                            # Session destruction may take time in a
                            # real-world example. Therefore the current time
                            # is computed for each iteration.
                            #
                            if (time.time() - p.session.timestamp()) > self._timeout:
                                name = p.proxy.getName()
                                p.proxy.destroy()
                                print "The session " + name + " has timed out."
                                self._sessions.remove(p)
                        except Ice.ObjectNotExistException:
                            self._sessions.remove(p)
        finally:
            self._cond.release()

    def terminate(self):
        self._cond.acquire()
        try:
            self._terminated = True
            self._cond.notify()

            self._sessions = []
        finally:
            self._cond.release()

    def add(self, proxy, session):
        self._cond.acquire()
        try:
            self._sessions.append(SessionProxyPair(proxy, session))
        finally:
            self._cond.release()

class SessionFactoryI(Demo.SessionFactory):
    def __init__(self, reaper):
        self._reaper = reaper
        self._lock = threading.Lock()

    def create(self, name, c):
        self._lock.acquire()
        try:
            session = SessionI(name)
            proxy = Demo.SessionPrx.uncheckedCast(c.adapter.addWithUUID(session))
            self._reaper.add(proxy, session)
            return proxy
        finally:
            self._lock.release()

    def shutdown(self, c):
        print "Shutting down..."
        c.adapter.getCommunicator().shutdown()

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("SessionFactory")
        reaper = ReapThread()
        reaper.start()
        try:
            adapter.add(SessionFactoryI(reaper), self.communicator().stringToIdentity("SessionFactory"))
            adapter.activate()
            self.communicator().waitForShutdown()
        finally:
            reaper.terminate()
            reaper.join()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
