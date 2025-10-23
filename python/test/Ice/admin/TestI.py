# Copyright (c) ZeroC, Inc.

import threading
from typing import Any, override

from generated.test.Ice.admin import Test

import Ice


def test(b: Any):
    if not b:
        raise RuntimeError("test assertion failed")


class TestFacetI(Test.TestFacet):
    @override
    def op(self, current: Ice.Current) -> None:
        return


class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self, communicator: Ice.Communicator):
        self.communicator = communicator
        self.called = False
        self.m = threading.Condition()

    @override
    def getAdmin(self, current: Ice.Current):
        return self.communicator.getAdmin()

    @override
    def getChanges(self, current: Ice.Current) -> dict[str, str]:
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

    @override
    def shutdown(self, current: Ice.Current) -> None:
        self.communicator.shutdown()

    @override
    def waitForShutdown(self, current: Ice.Current) -> None:
        #
        # Note that we are executing in a thread of the *main* communicator,
        # not the one that is being shut down.
        #
        self.communicator.waitForShutdown()

    @override
    def destroy(self, current: Ice.Current) -> None:
        self.communicator.destroy()

    def updated(self, changes: dict[str, str]) -> None:
        with self.m:
            self.changes = changes
            self.called = True
            self.m.notify()


class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):
    @override
    def createCommunicator(self, props: dict[str, str], current: Ice.Current) -> Test.RemoteCommunicatorPrx:
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
        communicator = Ice.Communicator(initData=init)

        #
        # Install a custom admin facet.
        #
        communicator.addAdminFacet(TestFacetI(), "TestFacet")

        #
        # Set the properties update callback on the admin facet.
        #
        servant = RemoteCommunicatorI(communicator)

        def properties_updated(changes: dict[str, str]) -> None:
            servant.updated(changes)

        admin = communicator.findAdminFacet("Properties")

        if admin is not None:
            assert isinstance(admin, Ice.NativePropertiesAdmin)
            admin.addUpdateCallback(properties_updated)

        return Test.RemoteCommunicatorPrx.uncheckedCast(current.adapter.addWithUUID(servant))

    @override
    def shutdown(self, current: Ice.Current) -> None:
        current.adapter.getCommunicator().shutdown()
