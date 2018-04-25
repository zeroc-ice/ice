# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestFacetI(Test.TestFacet):
    def op(self, current = None):
        return

class RemoteCommunicatorI(Test.RemoteCommunicator, Ice.PropertiesAdminUpdateCallback):
    def __init__(self, communicator):
        self.communicator = communicator
        self.called = False
        self.m = threading.Condition()

    def getAdmin(self, current = None):
        return self.communicator.getAdmin()

    def getChanges(self, current = None):
        with self.m:
            #
            # The client calls PropertiesAdmin::setProperties() and then invokes
            # this operation. Since setProperties() is implemented using AMD, the
            # client might receive its reply and then call getChanges() before our
            # updated() method is called. We block here to ensure that updated()
            # gets called before we return the most recent set of changes.
            #
            while not self.called:
                self.m.wait()

            self.called = False

            return self.changes

    def shutdown(self, current = None):
        self.communicator.shutdown()

    def waitForShutdown(self, current = None):
        #
        # Note that we are executing in a thread of the *main* communicator,
        # not the one that is being shut down.
        #
        self.communicator.waitForShutdown()

    def destroy(self, current = None):
        self.communicator.destroy()

    def updated(self, changes):
        with self.m:
            self.changes = changes
            self.called = True
            self.m.notify()

class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):

    def createCommunicator(self, props, current = None):
        #
        # Prepare the property set using the given properties.
        #
        init = Ice.InitializationData()
        init.properties = Ice.createProperties()
        for k, v in props.items():
            init.properties.setProperty(k, v)

        #
        # Initialize a new communicator.
        #
        communicator = Ice.initialize(init)

        #
        # Install a custom admin facet.
        #
        communicator.addAdminFacet(TestFacetI(), "TestFacet")

        #
        # The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
        # Set the callback on the admin facet.
        #
        servant = RemoteCommunicatorI(communicator)
        admin = communicator.findAdminFacet("Properties")
        if admin != None:
            admin.addUpdateCallback(servant)

        proxy = current.adapter.addWithUUID(servant)
        return Test.RemoteCommunicatorPrx.uncheckedCast(proxy)

    def shutdown(self, current = None):
        current.adapter.getCommunicator().shutdown()
