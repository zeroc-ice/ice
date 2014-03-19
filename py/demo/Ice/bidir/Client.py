#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, Ice

slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Callback.ice")
import Demo

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, num, current=None):
        print("received callback #" + str(num))

class Client(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        server = Demo.CallbackSenderPrx.checkedCast(self.communicator().propertyToProxy('CallbackSender.Proxy'))
        if not server:
            print(self.appName() + ": invalid proxy")
            return 1

        adapter = self.communicator().createObjectAdapter("")
        ident = Ice.Identity()
        ident.name = Ice.generateUUID()
        ident.category = ""
        adapter.add(CallbackReceiverI(), ident)
        adapter.activate()
        server.ice_getConnection().setAdapter(adapter)
        server.addClient(ident)
        self.communicator().waitForShutdown()

        return 0

app = Client()
sys.exit(app.main(sys.argv, "config.client"))
