#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, threading, Ice, Glacier2

Ice.loadSlice('Callback.ice')
import Demo

def menu():
    print """
usage:
t: send callback as twoway
o: send callback as oneway
O: send callback as batch oneway
f: flush all batch requests
v: set/reset override context field
F: set/reset fake category
s: shutdown server
x: exit
?: help
"""

class SessionRefreshThread(threading.Thread):
    def __init__(self, router, timeout):
        threading.Thread.__init__(self)
        self._router = router
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
                        self._router.ice_ping()
                    except Ice.LocalException, ex:
                        pass
        finally:
            self._cond.release()

    def terminate(self):
        self._cond.acquire()
        try:
            self._terminated = True
            self._cond.notify()
        finally:
            self._cond.release()

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, current=None):
        print "received callback"

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print self.appName() + ": too many arguments"
            return 1

        defaultRouter = self.communicator().getDefaultRouter()
        if not defaultRouter:
            print self.appName() + ": no default router set"
            return 1

        router = Glacier2.RouterPrx.checkedCast(defaultRouter)
        if not router:
            print self.appName() + ": configured router is not a Glacier2 router"
            return 1

        while True:
            print "This demo accepts any user-id / password combination."
            id = raw_input("user id: ")
            pw = raw_input("password: ")
            try:
                router.createSession(id, pw)
                break
            except Glacier2.PermissionDeniedException, ex:
                print "permission denied:\n" + ex.reason

        refresh = SessionRefreshThread(router, router.getSessionTimeout() / 2.0)
        refresh.start()

        category = router.getCategoryForClient()
        callbackReceiverIdent = Ice.Identity()
        callbackReceiverIdent.name = "callbackReceiver"
        callbackReceiverIdent.category = category
        callbackReceiverFakeIdent = Ice.Identity()
        callbackReceiverFakeIdent.name = "callbackReceiver"
        callbackReceiverFakeIdent.category = "fake"

        base = self.communicator().propertyToProxy('Callback.Proxy')
        twoway = Demo.CallbackPrx.checkedCast(base)
        oneway = Demo.CallbackPrx.uncheckedCast(twoway.ice_oneway())
        batchOneway = Demo.CallbackPrx.uncheckedCast(twoway.ice_batchOneway())

        adapter = self.communicator().createObjectAdapterWithRouter("Callback.Client", defaultRouter)
        adapter.add(CallbackReceiverI(), callbackReceiverIdent)
        adapter.add(CallbackReceiverI(), callbackReceiverFakeIdent)
        adapter.activate()

        twowayR = Demo.CallbackReceiverPrx.uncheckedCast(adapter.createProxy(callbackReceiverIdent))
        onewayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway())

        override = ''
        fake = False

        menu()

        c = None
        while c != 'x':
            try:
                c = raw_input("==> ")
                if c == 't':
                    context = {}
                    context["_fwd"] = "t"
                    if not len(override) == 0:
                        context["_ovrd"] = override
                    twoway.initiateCallback(twowayR, context)
                elif c == 'o':
                    context = {}
                    context["_fwd"] = "o"
                    if not len(override) == 0:
                        context["_ovrd"] = override
                    oneway.initiateCallback(onewayR, context)
                elif c == 'O':
                    context = {}
                    context["_fwd"] = "O"
                    if not len(override) == 0:
                        context["_ovrd"] = override
                    batchOneway.initiateCallback(onewayR, context)
                elif c == 'f':
                    self.communicator().flushBatchRequests()
                elif c == 'v':
                    if len(override) == 0:
                        override = "some_value"
                        print "override context field is now `" + override + "'"
                    else:
                        override = ''
                        print "override context field is empty"
                elif c == 'F':
                    fake = not fake

                    if fake:
                        twowayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_identity(callbackReceiverFakeIdent))
                        onewayR = Demo.CallbackReceiverPrx.uncheckedCast(onewayR.ice_identity(callbackReceiverFakeIdent))
                    else:
                        twowayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_identity(callbackReceiverIdent))
                        onewayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_identity(callbackReceiverIdent))
                elif c == 's':
                    twoway.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print "unknown command `" + c + "'"
                    menu()
            except KeyboardInterrupt:
                break
            except EOFError:
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

        try:
            router.destroySession()
        except Glacier2.SessionNotExistException, ex:
            print ex
        except Ice.ConnectionLostException:
            # Expected: the router closed the connection.
            pass

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
