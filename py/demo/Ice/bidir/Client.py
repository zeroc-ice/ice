#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, Ice

slice_dir = os.getenv('ICEPY_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, 'slice')):
    slice_dir = os.getenv('ICE_HOME', '')
if len(slice_dir) == 0 or not os.path.exists(os.path.join(slice_dir, 'slice')):
    slice_dir = os.path.join('/', 'usr', 'share', 'Ice-3.2.0')
if not os.path.exists(os.path.join(slice_dir, 'slice')):
    print sys.argv[0] + ': Slice directory not found. Define ICEPY_HOME or ICE_HOME.'
    sys.exit(1)

Ice.loadSlice('-I' + slice_dir + '/slice Callback.ice')
import Demo

class CallbackReceiverI(Demo.CallbackReceiver):
    def callback(self, num, current=None):
        print "received callback #" + str(num)

class Client(Ice.Application):
    def run(self, args):
        server = Demo.CallbackSenderPrx.checkedCast(self.communicator().propertyToProxy('Callback.Client.CallbackServer'))
        if not server:
            print self.appName() + ": invalid proxy"
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
