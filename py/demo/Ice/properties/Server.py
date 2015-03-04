#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import sys, traceback, Ice, threading

slice_dir = Ice.getSliceDir()
if not slice_dir:
    print(sys.argv[0] + ': Slice directory not found.')
    sys.exit(1)

Ice.loadSlice("'-I" + slice_dir + "' Props.ice")
import Demo

class PropsI(Demo.Props, Ice.PropertiesAdminUpdateCallback):
    def __init__(self):
        self.called = False
        self.m = threading.Condition()

    def getChanges(self, current = None):
        self.m.acquire()
        try:
            #
            # Make sure that we have received the property updates before we
            # return the results.
            #
            while not self.called:
                self.m.wait()
            self.called = False;
            return self.changes
        finally:
            self.m.release()
        
    def shutdown(self, current = None):
        current.adapter.getCommunicator().shutdown();

    def updated(self, changes):
        self.m.acquire()
        try:
            self.changes = changes;
            self.called = True
            self.m.notify()
        finally:
            self.m.release()

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        servant = PropsI()
 
        #
        # Retrieve the PropertiesAdmin facet and register the servant as the update callback.
        #
        admin = self.communicator().findAdminFacet("Properties");
        admin.addUpdateCallback(servant);

        adapter = self.communicator().createObjectAdapter("Props")
        adapter.add(servant, self.communicator().stringToIdentity("props"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv, "config.server"))
