#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice

Ice.loadSlice('Callback.ice')
import Demo

def menu():
    print("""
usage:
t: send callback
s: shutdown server
x: exit
?: help
""")

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, current=None):
        print("received callback")

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        sender = Demo.CallbackSenderPrx.checkedCast(
            self.communicator().propertyToProxy('CallbackSender.Proxy').
            ice_twoway().ice_timeout(-1).ice_secure(False))
        if not sender:
            print(self.appName() + ": invalid proxy")
            return 1

        adapter = self.communicator().createObjectAdapter("Callback.Client")
        adapter.add(CallbackReceiverI(), self.communicator().stringToIdentity("callbackReceiver"))
        adapter.activate()

        receiver = Demo.CallbackReceiverPrx.uncheckedCast(
            adapter.createProxy(self.communicator().stringToIdentity("callbackReceiver")))

        menu()

        c = None
        while c != 'x':
            try:
                sys.stdout.write("==> ")
                sys.stdout.flush()
                c = sys.stdin.readline().strip()
                if c == 't':
                    sender.initiateCallback(receiver)
                elif c == 's':
                    sender.shutdown()
                elif c == 'x':
                    pass # Nothing to do
                elif c == '?':
                    menu()
                else:
                    print("unknown command `" + c + "'")
                    menu()
            except EOFError:
                break
            except KeyboardInterrupt:
                break

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
