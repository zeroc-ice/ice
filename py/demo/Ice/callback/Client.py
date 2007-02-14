#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, Ice

Ice.loadSlice('Callback.ice')
import Demo

def menu():
    print """
usage:
t: send callback as twoway
o: send callback as oneway
O: send callback as batch oneway
d: send callback as datagram
D: send callback as batch datagram
f: flush all batch requests
S: switch secure mode on/off
s: shutdown server
x: exit
?: help
"""

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, current=None):
        print "received callback"

class Client(Ice.Application):
    def run(self, args):
        base = self.communicator().propertyToProxy('Callback.CallbackServer')
        twoway = Demo.CallbackSenderPrx.checkedCast(base.ice_twoway().ice_timeout(-1).ice_secure(False))
        if not twoway:
            print self.appName() + ": invalid proxy"
            return 1

        oneway = Demo.CallbackSenderPrx.uncheckedCast(twoway.ice_oneway())
        batchOneway = Demo.CallbackSenderPrx.uncheckedCast(twoway.ice_batchOneway())
        datagram = Demo.CallbackSenderPrx.uncheckedCast(twoway.ice_datagram())
        batchDatagram = Demo.CallbackSenderPrx.uncheckedCast(twoway.ice_batchDatagram())

        adapter = self.communicator().createObjectAdapter("Callback.Client")
        adapter.add(CallbackReceiverI(), self.communicator().stringToIdentity("callbackReceiver"))
        adapter.activate()

        twowayR = Demo.CallbackReceiverPrx.uncheckedCast(
            adapter.createProxy(self.communicator().stringToIdentity("callbackReceiver")))
        onewayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_oneway())
        datagramR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_datagram())

        secure = False
        secureStr = ''

        menu()

        c = None
        while c != 'x':
            try:
                c = raw_input("==> ")
                if c == 't':
                    twoway.initiateCallback(twowayR)
                elif c == 'o':
                    oneway.initiateCallback(onewayR)
                elif c == 'O':
                    batchOneway.initiateCallback(onewayR)
                elif c == 'd':
                    if secure:
                        print "secure datagrams are not supported"
                    else:
                        datagram.initiateCallback(datagramR)
                elif c == 'D':
                    if secure:
                        print "secure datagrams are not supported"
                    else:
                        batchDatagram.initiateCallback(datagramR)
                elif c == 'f':
                    self.communicator().flushBatchRequests()
                elif c == 'S':
                    secure = not secure

                    twoway = Demo.CallbackSenderPrx.uncheckedCast(twoway.ice_secure(secure))
                    oneway = Demo.CallbackSenderPrx.uncheckedCast(oneway.ice_secure(secure))
                    batchOneway = Demo.CallbackSenderPrx.uncheckedCast(batchOneway.ice_secure(secure))
                    datagram = Demo.CallbackSenderPrx.uncheckedCast(datagram.ice_secure(secure))
                    batchDatagram = Demo.CallbackSenderPrx.uncheckedCast(batchDatagram.ice_secure(secure))

                    twowayR = Demo.CallbackReceiverPrx.uncheckedCast(twowayR.ice_secure(secure))
                    onewayR = Demo.CallbackReceiverPrx.uncheckedCast(onewayR.ice_secure(secure))
                    datagramR = Demo.CallbackReceiverPrx.uncheckedCast(datagramR.ice_secure(secure))

                    if secure:
                        print "secure mode is now on"
                    else:
                        print "secure mode is now off"
                elif c == 's':
                    twoway.shutdown()
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

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
