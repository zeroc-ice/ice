# Copyright (c) ZeroC, Inc.

import Ice
import Test
import threading


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestFacetI(Test.TestFacet):
    def op(self, current):
        return


class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self, communicator):
        self.communicator = communicator
        self.called = False
        self.m = threading.Condition()

    def getAdmin(self, current):
        return self.communicator.getAdmin()

    def getChanges(self, current):
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

    def shutdown(self, current):
        self.communicator.shutdown()

    def waitForShutdown(self, current):
        #
        # Note that we are executing in a thread of the *main* communicator,
        # not the one that is being shut down.
        #
        self.communicator.waitForShutdown()

    def destroy(self, current):
        self.communicator.destroy()

    def updated(self, changes):
        with self.m:
            self.changes = changes
            self.called = True
            self.m.notify()


class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):
    def createCommunicator(self, props, current):
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
        communicator = Ice.initialize(initData=init)

        #
        # Install a custom admin facet.
        #
        communicator.addAdminFacet(TestFacetI(), "TestFacet")

        #
        # Set the properties update callback on the admin facet.
        #
        servant = RemoteCommunicatorI(communicator)

        def properties_updated(changes):
            servant.updated(changes)
        admin = communicator.findAdminFacet("Properties")
        if admin is not None:
            admin.addUpdateCallback(properties_updated)

        return current.adapter.addWithUUID(servant)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()
