#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, time, traceback, threading, Ice

Ice.loadSlice('Session.ice')
import Demo

class HelloI(Demo.Hello):
    def __init__(self, id):
        self._id = id

    def sayHello(self, current=None):
        print str(self._id) + ": Hello World!"

class SessionI(Demo.Session):
    def __init__(self, adapter, timeout):
	self._adapter = adapter
	self._timeout = timeout
	self._refreshTime = time.clock()
        self._lock = threading.Lock()
        self._destroy = False # true if destroy() was called, false otherwise.
        self._refreshTime = 0 # The last time the session was refreshed.
        self._nextId = 0 # The id of the next hello object. This is used for tracing purposes.
        self._objs = [] # List of per-client allocated Hello objects.
    
    def createHello(self, current=None):
        self._lock.acquire()
        try:
            hello = Demo.HelloPrx.uncheckedCast(self._adapter.addWithUUID(HelloI(self._nextId)))
            self._nextId = self._nextId + 1
            self._objs.append(hello)
            return hello
        finally:
	    self._lock.release()

    def refresh(self, current=None):
        self._lock.acquire()
        try:
            self._refreshTime = time.clock()
        finally:
	    self._lock.release()
    
    def destroy(self, current=None):
        self._lock.acquire()
        try:
            self._destroy = True
        finally:
	    self._lock.release()
    
    # Return true if the session is destroyed, false otherwise.
    def destroyed(self):
        self._lock.acquire()
        try:
            return self._destroy or (time.clock() - self._refreshTime) > self._timeout
        finally:
	    self._lock.release()

    # Called when the session is destroyed. This should release any
    # per-client allocated resources.
    def destroyCallback(self):
        self._lock.acquire()
        try:
            print "SessionI.destroyCallback: _destroy=" + str(self._destroy) + " timeout=" + \
                               str((time.clock() - self._refreshTime) > self._timeout)
            for h in self._objs:
                try:
                    self._adapter.remove(h.ice_getIdentity())
                except:
                    # This method is called on shutdown of the server, in
                    # which case this exception is expected.
                    pass
            self._objs = []
        finally:
	    self._lock.release()

class ReapThread(threading.Thread):
    def __init__(self, factory, timeout):
        threading.Thread.__init__(self)
        self._timeout = timeout
        self._factory = factory
        self._terminated = False
        self._cond = threading.Condition()

    def run(self):
        self._cond.acquire()
        try:
            while not self._terminated:
                self._cond.wait(self._timeout)
                if not self._terminated:
                    self._factory.reap()
        finally:
	    self._cond.release()

    def terminate(self):
	self._cond.acquire()
	try:
            self._terminated = True
	    self._cond.notify()
	finally:
	    self._cond.release()

class SessionId:
    def __init__(self, s, i):
        self.session = s
        self.id = i

class SessionFactoryI(Demo.SessionFactory):
    def __init__(self, adapter):
	self._adapter = adapter
        self._timeout = 10 # 10 seconds
        self._sessions = []
        self._lock = threading.Lock()
	self._reapThread = ReapThread(self, self._timeout)
	self._reapThread.start()

    def create(self, current=None):
        self._lock.acquire()
        try:
            session = SessionI(self._adapter, self._timeout)
            proxy = Demo.SessionPrx.uncheckedCast(self._adapter.addWithUUID(session))
            self._sessions.append(SessionId(session, proxy.ice_getIdentity()))
            return proxy
        finally:
            self._lock.release()

    def shutdown(self, current=None):
	print "Shutting down..."
	current.adapter.getCommunicator().shutdown()

    def destroy(self):
        self._lock.acquire()
        try:
            self._reapThread.terminate()
            self._reapThread.join()

            for s in self._sessions:
                s.session.destroyCallback()

                # When the session factory is destroyed the OA is
                # deactivated and all servants have been removed so
                # calling remove on the OA is not necessary.
            self._sessions = []
        finally:
            self._lock.release()

    def reap(self):
        self._lock.acquire()
        try:
            for s in self._sessions:
                if s.session.destroyed():
                    s.session.destroyCallback()
                    self._adapter.remove(s.id)
                    self._sessions.remove(s)
        finally:
            self._lock.release()

def run(argv, communicator):
    adapter = communicator.createObjectAdapter("SessionFactory")
    factory = SessionFactoryI(adapter)
    try:
        adapter.add(factory, Ice.stringToIdentity("SessionFactory"))
        adapter.activate()
        communicator.waitForShutdown()
    finally:
        factory.destroy()
    return True

try:
    properties = Ice.createProperties()
    properties.load("config")
    communicator = Ice.initializeWithProperties(sys.argv, properties)
    status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

if communicator:
    try:
        communicator.destroy()
    except:
        traceback.print_exc()
        status = False

if status:
    sys.exit(0)
else:
    sys.exit(1)
