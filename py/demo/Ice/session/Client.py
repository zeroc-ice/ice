#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, threading, Ice

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
                        self._session.refresh()
                    except Ice.LocalException, ex:
                        self._logger.warning("SessionRefreshThread: " + str(ex))
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

class Client(Ice.Application):
    def run(self, args):
        while True:
            name = raw_input("Please enter your name ==> ").strip()
            if len(name) != 0:
                break

        base = self.communicator().propertyToProxy('SessionFactory.Proxy')
        factory = Demo.SessionFactoryPrx.checkedCast(base)
        if not factory:
            print args[0] + ": invalid proxy"
            return 1

        session = factory.create(name)
        try:
            refresh = SessionRefreshThread(self.communicator().getLogger(), 5, session)
            refresh.start()

            hellos = []

            self.menu()

            destroy = True
            shutdown = False
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
                            print "Index is too high. " + str(len(hellos)) + " hello objects exist so far.\n" +\
                                  "Use `c' to create a new hello object."
                    elif c == 'c':
                        hellos.append(session.createHello())
                        print "Created hello object",len(hellos) - 1
                    elif c == 's':
                        destroy = False
                        shutdown = True
                        break
                    elif c == 'x':
                        break
                    elif c == 't':
                        destroy = False
                        break
                    elif c == '?':
                        self.menu()
                    else:
                        print "unknown command `" + c + "'"
                        self.menu()
                except EOFError:
                    break
                except KeyboardInterrupt:
                    break
            #
            # The refresher thread must be terminated before destroy is
            # called, otherwise it might get ObjectNotExistException. refresh
            # is set to 0 so that if session->destroy() raises an exception
            # the thread will not be re-terminated and re-joined.
            #
            refresh.terminate()
            refresh.join()
            refresh = None

            if destroy:
                session.destroy()
            if shutdown:
                factory.shutdown()
        finally:
            #
            # The refresher thread must be terminated in the event of a
            # failure.
            #
            if refresh != None:
                refresh.terminate()
                refresh.join()

        return 0

    def menu(self):
        print """
usage:
c:     create a new per-client hello object
0-9:   send a greeting to a hello object
s:     shutdown the server and exit
x:     exit
t:     exit without destroying the session
?:     help
"""

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
