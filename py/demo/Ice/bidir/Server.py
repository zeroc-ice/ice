#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback, threading, Ice

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

class CallbackSenderI(Demo.CallbackSender, threading.Thread):
    def __init__(self, communicator):
        threading.Thread.__init__(self)
        self._communicator = communicator
        self._destroy = False
        self._num = 0
        self._clients = []
        self._cond = threading.Condition()

    def destroy(self):
        self._cond.acquire()

        print "destroying callback sender"
        self._destroy = True

        try:
            self._cond.notify()
        finally:
            self._cond.release()

        self.join()

    def addClient(self, ident, current=None):
        self._cond.acquire()

        print "adding client `" + self._communicator.identityToString(ident) + "'"

        client = Demo.CallbackReceiverPrx.uncheckedCast(current.con.createProxy(ident))
        self._clients.append(client)

        self._cond.release()

    def run(self):
        self._cond.acquire()

        try:
            while not self._destroy:
                self._cond.wait(2)

                if not self._destroy and len(self._clients) > 0:
                    self._num = self._num + 1

                    for p in self._clients[:]: # Iterate over a copy so we can modify the original list.
                        try:
                            p.callback(self._num)
                        except:
                            print "removing client `" + self._communicator.identityToString(p.ice_getIdentity()) + "':"
                            traceback.print_exc()
                            self._clients.remove(p)
        finally:
            self._cond.release()

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Callback.Server")
        sender = CallbackSenderI(self.communicator())
        adapter.add(sender, self.communicator().stringToIdentity("sender"))
        adapter.activate()

        sender.start()
        try:
            self.communicator().waitForShutdown()
        finally:
            sender.destroy()

        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
