#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, threading, Ice, Glacier2
Ice.loadSlice('Callback.ice')
import Demo

def menu():
    print("""
usage:
t: send callback as twoway
o: send callback as oneway
O: send callback as batch oneway
f: flush all batch requests
v: set/reset override context field
F: set/reset fake category
s: shutdown server
r: restart the session
x: exit
?: help
""")

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, current=None):
        print("received callback")

class Client(Glacier2.Application):
    def createSession(self):
        session = None
        while True:
            print("This demo accepts any user-id / password combination.")
            sys.stdout.write("user id: ")
            sys.stdout.flush()
            id = sys.stdin.readline().strip()
            sys.stdout.write("password: ")
            sys.stdout.flush()
            pw = sys.stdin.readline().strip()
            try:
                session = self.router().createSession(id, pw)
                break
            except Glacier2.PermissionDeniedException as ex:
                print("permission denied:\n" + ex.reason)
            except Glacier2.CannotCreateSessionException as ex:
                print("cannot create session:\n" + ex.reason)
        return session

    def runWithSession(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        callbackReceiverIdent = self.createCallbackIdentity("callbackReceiver")

        callbackReceiverFakeIdent = Ice.Identity()
        callbackReceiverFakeIdent.name = "callbackReceiver"
        callbackReceiverFakeIdent.category = "fake"

        base = self.communicator().propertyToProxy('Callback.Proxy')
        twoway = Demo.CallbackPrx.checkedCast(base)
        oneway = Demo.CallbackPrx.uncheckedCast(twoway.ice_oneway())
        batchOneway = Demo.CallbackPrx.uncheckedCast(twoway.ice_batchOneway())

        self.objectAdapter().add(CallbackReceiverI(), callbackReceiverIdent)

        # Should never be called for the fake identity.
        self.objectAdapter().add(CallbackReceiverI(), callbackReceiverFakeIdent)

        twowayR = Demo.CallbackReceiverPrx.uncheckedCast(self.objectAdapter().createProxy(callbackReceiverIdent))
        onewayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway())

        override = ''
        fake = False

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
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
                        print("override context field is now `" + override + "'")
                    else:
                        override = ''
                        print("override context field is empty")
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
                elif c == 'r':
                    self.restart()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print("unknown command `" + c + "'")
                    menu()
            except KeyboardInterrupt:
                break
            except EOFError:
                break

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
