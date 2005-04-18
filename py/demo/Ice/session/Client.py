#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, threading, Ice

Ice.loadSlice('Session.ice')
import Demo

class SessionRefreshThread(threading.Thread):
    def __init__(self, logger, timeout, session):
        threading.Thread.__init__(self)
        self._logger = logger
        self._session = session
        self._timeout = timeout
        self._terminated = False
        self._cond = threading.Condition()

    def run(self):
        self._cond.acquire()
        try:
            while not self._terminated:
                self._cond.wait(self._timeout)
		if not self._terminated:
                    try:
                        print "refresh!"
                        self._session.refresh()
                    except:
                        _logger.warning("SessionRefreshThread: " + traceback.format_exc())
                        self._terminated = True
	finally:
	    self._cond.release()

    def terminate(self):
	self._cond.acquire()
	try:
            self._terminated = True
	    self._cond.notify()
	finally:
	    self._cond.release()

def menu():
    print """
usage:
c:     create new hello
0-9:   greet hello object
s:     shutdown server
x:     exit
t:     exit without destroying the session
?:     help
"""

def run(args, communicator):
    properties = communicator.getProperties()
    refProperty = 'SessionFactory.Proxy'
    proxy = properties.getProperty(refProperty)
    if len(proxy) == 0:
        print args[0] + ": property `" + refProperty + "' not set"
        return False

    base = communicator.stringToProxy(proxy)
    factory = Demo.SessionFactoryPrx.checkedCast(base)
    if not factory:
        print args[0] + ": invalid proxy"
        return False

    try:
        session = factory.create()
        refresh = SessionRefreshThread(communicator.getLogger(), 5, session)
        refresh.start()

        hellos = []

        menu()

        c = None
        destroy = True
        while True:
            try:
                c = raw_input("==> ")
                s = str(c)
                if s.isdigit():
		    index = int(s)
                    if index < len(hellos):
			hello = hellos[index]
			hello.sayHello()
		    else:
			print "index is too high. " + str(len(hellos)) + " exist so far. " +\
                              "Use 'c' to create a new hello object."
                elif c == 'c':
                    hellos.append(session.createHello())
                elif c == 's':
                    factory.shutdown()
                    break
                elif c == 'x':
                    break
                elif c == 't':
                    destroy = False
                    break
                elif c == '?':
                    menu()
                else:
                    print "unknown command `" + c + "'"
                    menu()
            except EOFError:
                break
        if destroy:
            session.destroy()
    finally:
        refresh.terminate()
        refresh.join()

    return True

communicator = None
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

sys.exit(not status)
